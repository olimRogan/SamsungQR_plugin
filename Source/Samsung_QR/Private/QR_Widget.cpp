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
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

void UQR_Widget::NativeConstruct()
{
	Super::NativeConstruct();

	// QR 이미지 설정
	QR_Image->SetBrushFromTexture(LoadQRTexture_FromFile());

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
				if(DataActor)
				{
					DataActor->CurrentData.ID = GetCustomerID(str);
					DataActor->CurrentData.Center = GetCenterName();
					DataActor->CurrentData.LogInTime = GetCurrentTime();
				}
			}

			OnComplete(FHttpServerResponse::Ok());

			return true;
		});

	bClosed = false;

	// Data Actor 생성
	DataActor = GetWorld()->SpawnActor<ADataActor>();
	if (DataActor) { UE_LOG(LogTemp, Warning, TEXT("DataActor")) };

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
	if(DataActor)
	{
		DataActor->Destroy();
	}

	bClosed = true;
}

bool UQR_Widget::CustomerJsonFileCheck()
{
	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/"+ GetCustomerFileName();

	IFileManager& FileManager = IFileManager::Get();

	return (FileManager.FileExists(*(Path)));
}

FString UQR_Widget::GetCustomerFileName()
{
	// 현재 년, 월, 일 받아오기
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();

	const FString FileName = FString::Printf(TEXT("%d_%d_%d.json"), Year, Month, Day);
	return FileName;
}

FString UQR_Widget::GetCenterName()
{
	const FString FullPathFromRoot = FPaths::ProjectConfigDir() + "ClientConfig.json";

	FString StringToLoad;
	if(FFileHelper::LoadFileToString(StringToLoad, *FullPathFromRoot))
	{
		FString Ret;
		const TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(StringToLoad);
		TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

		if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
		{
			Ret = JsonObject->GetStringField(TEXT("CenterName"));
			return Ret;
		}
	}
	return FString();
}

FString UQR_Widget::GetCustomerID(const FString& Str)
{
	FString RString;
	Str.Split("ID\" : \"", nullptr, &RString);
	FString ID;
	RString.Split("\"", &ID, nullptr);

	return ID;
}

FString UQR_Widget::GetCurrentTime()
{
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();
	const int32 Hour = UTCTime.GetHour() + 9;
	const int32 Minute = UTCTime.GetMinute();
	const int32 Second = UTCTime.GetSecond();

	return FString::Printf(TEXT("%d_%d_%d_%02d_%02d_%02d"), Year, Month, Day, Hour, Minute, Second);
}

void UQR_Widget::OnClickedSkipBtn()
{
	if(DataActor)
	{
		DataActor->CurrentData.ID = "unknown";
		DataActor->CurrentData.Center = GetCenterName();
		DataActor->CurrentData.LogInTime = GetCurrentTime();
	}
}

void UQR_Widget::OnClickedExitBtn()
{
	if(DataActor)
	{
		// Json Object 생성
		FString JsonStr;
		const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

		if(DataActor)
		{
			// Arr 에 추가 후 초기화
			DataActor->CustomerDataArr.Emplace(DataActor->CurrentData);
			DataActor->CurrentData = FCustomer();

			JsonObj->WriteObjectStart();
			JsonObj->WriteObjectStart(TEXT("CustomerInfo"));
			JsonObj->WriteArrayStart(TEXT("Customer"));

			for (const auto& Data : DataActor->CustomerDataArr)
			{
				JsonObj->WriteObjectStart();
				JsonObj->WriteValue(TEXT("id"), Data.ID);				// ID
				JsonObj->WriteValue(TEXT("center"), GetCenterName());	// 매장
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
