// Fill out your copyright notice in the Description page of Project Settings.

#include "QR_Widget.h"

#include "HTTPServer/Public/HttpServerModule.h"
#include "HTTPServer/Public/IHttpRouter.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Components/Image.h"
#include "HTTPServer/Private/HttpRequestHandlerRegistrar.h"
#include "HTTPServer/Public/HttpServerResponse.h"

void UQR_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	QR_Image->SetBrushFromTexture(LoadQRTexture_FromFile());

	StartServer();
}

void UQR_Widget::NativeDestruct()
{
	StopServer();

	Super::NativeDestruct();
}

void UQR_Widget::StartServer()
{
	Server = &FHttpServerModule::Get();

	Router = Server->GetHttpRouter(ServerPort);

	RouteHandle = Router->BindRoute(
		HttpPath,
		EHttpServerRequestVerbs::VERB_GET | EHttpServerRequestVerbs::VERB_POST,
		[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {
			const FUTF8ToTCHAR WByteBuffer(reinterpret_cast<const ANSICHAR*>(Request.Body.GetData()), Request.Body.Num());
			const FString str = WByteBuffer.Get();

			UE_LOG(LogTemp, Log, TEXT("%s"), *str);

			for (auto param : Request.QueryParams)
			{
				UE_LOG(LogTemp, Log, TEXT("[Key] : %s || [Value] : %s"), *param.Key, *param.Value);
			}

			if (OnReceiveComplete.IsBound())
			{
				OnReceiveComplete.Broadcast();
			}

			OnComplete(FHttpServerResponse::Ok());

			return true;
		});

	Server->StartAllListeners();
}

void UQR_Widget::StopServer()
{
	if (bClosed)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Stop HTTP"));
	Server->StopAllListeners();

	if (Router)
	{
		Router->UnbindRoute(RouteHandle);
	}

	bClosed = true;
}

UTexture2D* UQR_Widget::LoadQRTexture_FromFile()
{
	UTexture2D* LoadedT2D = nullptr;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	TArray<uint8> RawFileData;

	FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/QR_code.png";

	UE_LOG(LogTemp, Warning, TEXT("%s"), *Path);

	if (!FFileHelper::LoadFileToArray(RawFileData, *Path))
	{
		UE_LOG(LogTemp, Warning, TEXT("failed loadtexture2d from file"));
		return nullptr;
	}

	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
	{
		TArray<uint8> UncompressedBRGA;

		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBRGA))
		{
			LoadedT2D = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			if (!LoadedT2D)
			{
				return nullptr;
			}
			void* TextureData = LoadedT2D->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBRGA.GetData(), UncompressedBRGA.Num());
			LoadedT2D->PlatformData->Mips[0].BulkData.Unlock();

			LoadedT2D->UpdateResource();

			UE_LOG(LogTemp, Warning, TEXT("success loadtexture2d from file"));
		}
	}
	return LoadedT2D;
}
