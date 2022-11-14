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
#include "Kismet/KismetMathLibrary.h"
#include "Samsung_QR/CustomerDataInstance/CustomerDataInstance.h"

void UQR_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	// QR 이미지 설정
	QR_Image->SetBrushFromTexture(LoadQRTexture_FromFile());

	DataInstance = GetGameInstance()->GetSubsystem<UCustomerDataInstance>();

	StartServer();
}

void UQR_Widget::NativeDestruct()
{
	StopServer();

	Super::NativeDestruct();
}

// Http Server Start
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
				if(DataInstance)
				{
					DataInstance->CurrentData.ID = UCustomerDataInstance::GetCustomerID(str);
					DataInstance->CurrentData.Center = UCustomerDataInstance::GetCenterName();
					DataInstance->CurrentData.LogInTime = UCustomerDataInstance::GetCurrentTime();

					DataInstance->LoginOrSkipCallback.ExecuteIfBound();
				}
			}

			OnComplete(FHttpServerResponse::Ok());

			return true;
		});

	bClosed = false;

	Server->StartAllListeners();
}

// Http Server Stop
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

// Json 파일 체크
bool UQR_Widget::CustomerJsonFileCheck()
{
	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/"+ GetCustomerFileName();

	IFileManager& FileManager = IFileManager::Get();

	return (FileManager.FileExists(*(Path)));
}

// 현재 년, 월, 일 받아오기
FString UQR_Widget::GetCustomerFileName()
{
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();

	const FString FileName = FString::Printf(TEXT("%d_%d_%d.json"), Year, Month, Day);
	return FileName;
}

// Skip 버튼 이벤트
void UQR_Widget::OnClickedSkipBtn()
{
	if(DataInstance)
	{
		DataInstance->CurrentData.ID = "unknown";
		DataInstance->CurrentData.Center = UCustomerDataInstance::GetCenterName();
		DataInstance->CurrentData.LogInTime = UCustomerDataInstance::GetCurrentTime();

		DataInstance->LoginOrSkipCallback.ExecuteIfBound();
	}
}

// Exit 버튼 이벤트
void UQR_Widget::OnClickedExitBtn()
{
	if(DataInstance)
	{
		// Json Object 생성
		FString JsonStr;
		const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

		if(DataInstance)
		{
			// 고객데이터 배열에 추가 후 현재 고객데이터 초기화
			DataInstance->CustomerDataArr.Emplace(DataInstance->CurrentData);
			DataInstance->CurrentData = FCustomer();

			JsonObj->WriteObjectStart();
			JsonObj->WriteObjectStart(TEXT("CustomerInfo"));
			JsonObj->WriteArrayStart(TEXT("Customer"));

			for (const auto& Data : DataInstance->CustomerDataArr)
			{
				JsonObj->WriteObjectStart();
				JsonObj->WriteValue(TEXT("id"), Data.ID);				// ID
				JsonObj->WriteValue(TEXT("center"), UCustomerDataInstance::GetCenterName());	// 매장
				JsonObj->WriteValue(TEXT("loginTime"), Data.LogInTime);	// 로그인 시간


				JsonObj->WriteArrayStart(TEXT("model"));						// 고른 제품
				for (const auto& Idx : Data.PickingModel)
				{
					JsonObj->WriteObjectStart();
					JsonObj->WriteValue(TEXT("code"), Idx.Key);			// 제품 코드
					JsonObj->WriteValue(TEXT("time"), Idx.Value);				// 일시
					JsonObj->WriteObjectEnd();
				}
				JsonObj->WriteArrayEnd();

				JsonObj->WriteObjectEnd();
			}
			JsonObj->WriteArrayEnd();
			JsonObj->WriteObjectEnd();
			JsonObj->WriteObjectEnd();
			JsonObj->Close();

			const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCustomerFileName();
			FFileHelper::SaveStringToFile(*JsonStr, *Path);
		}
	}
}

// 로컬에서 QR 이미지를 Texture2D 로 런타임 중 로드한다.
UTexture2D* UQR_Widget::LoadQRTexture_FromFile()
{
	UTexture2D* LoadedT2D = nullptr;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	TArray<uint8> RawFileData;

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/QR_code.png";

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
