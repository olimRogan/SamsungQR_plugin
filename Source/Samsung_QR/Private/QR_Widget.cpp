// Fill out your copyright notice in the Description page of Project Settings.

#include "QR_Widget.h"

#include "CustomerDataInstance.h"
#include "HttpServerModule.h"
#include "HttpServerResponse.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"


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

//// Json 파일 체크
//bool UQR_Widget::CustomerJsonFileCheck()
//{
//	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/"+ GetCustomerFileName();
//
//	IFileManager& FileManager = IFileManager::Get();
//
//	return (FileManager.FileExists(*(Path)));
//}
//
//// 현재 년, 월, 일 받아오기
//FString UQR_Widget::GetCustomerFileName()
//{
//	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();
//
//	const int32 Year = UTCTime.GetYear();
//	const int32 Month = UTCTime.GetMonth();
//	const int32 Day = UTCTime.GetDay();
//
//	const FString FileName = FString::Printf(TEXT("%d_%d_%d.json"), Year, Month, Day);
//	return FileName;
//}

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
	// CurrentData 초기화
	if(DataInstance)
	{
		DataInstance->CurrentData = FCustomer();
	}
	/*
	if(datainstance)
	{
		// json object 생성
		fstring jsonstr;
		const tsharedref<tjsonwriter<tchar>> jsonobj = tjsonwriterfactory<>::create(&jsonstr);

		if(datainstance)
		{
			// 고객데이터 배열에 추가 후 현재 고객데이터 초기화
			datainstance->customerdataarr.emplace(datainstance->currentdata);
			datainstance->currentdata = fcustomer();

			jsonobj->writeobjectstart();
			jsonobj->writeobjectstart(text("customerinfo"));
			jsonobj->writearraystart(text("customer"));

			for (const auto& data : datainstance->customerdataarr)
			{
				jsonobj->writeobjectstart();
				jsonobj->writevalue(text("id"), data.id);				// id
				jsonobj->writevalue(text("center"), ucustomerdatainstance::getcentername());	// 매장
				jsonobj->writevalue(text("logintime"), data.logintime);	// 로그인 시간


				jsonobj->writearraystart(text("model"));						// 고른 제품
				for (const auto& idx : data.pickingmodel)
				{
					jsonobj->writeobjectstart();
					jsonobj->writevalue(text("code"), idx.key);			// 제품 코드
					jsonobj->writevalue(text("time"), idx.value);				// 일시
					jsonobj->writeobjectend();
				}
				jsonobj->writearrayend();

				jsonobj->writeobjectend();
			}
			jsonobj->writearrayend();
			jsonobj->writeobjectend();
			jsonobj->writeobjectend();
			jsonobj->close();

			const fstring path = fpaths::projectpluginsdir() + "samsung_qr/content/db/" + getcustomerfilename();
			ffilehelper::savestringtofile(*jsonstr, *path);
		}
	}*/
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
