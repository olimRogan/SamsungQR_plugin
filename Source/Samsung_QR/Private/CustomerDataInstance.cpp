// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomerDataInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UCustomerDataInstance::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);


	// 델리게이트 바인딩
	LoginOrSkipCallback.BindUObject(this, &UCustomerDataInstance::LoginOrSkipCallbackFunction);
}

FString UCustomerDataInstance::GetCurrentTime()
{
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();
	const int32 Hour = UTCTime.GetHour() + 9;
	const int32 Minute = UTCTime.GetMinute();

	const int32 Second = UTCTime.GetSecond();


	return FString::Printf(TEXT("%d-%d-%d_%02d:%02d:%02d"), Year, Month, Day, Hour, Minute, Second);
}

FString UCustomerDataInstance::GetCurrentYearMonthDay()
{
	// 현재 년, 월, 일 받아오기
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();

	return FString::Printf(TEXT("%d-%d-%d.json"), Year, Month, Day);
}

//void UCustomerDataInstance::CreateDB(FCustomer Customer,TArray<FCustomer> Customers)
//{
//	// Json Object 생성
//	FString JsonStr;
//	const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);
//
//	// Arr 에 추가 후 초기화
//	Customers.Emplace(Customer);
//	Customer = FCustomer();
//
//	JsonObj->WriteObjectStart();
//	JsonObj->WriteObjectStart(TEXT("CustomerInfo"));
//	JsonObj->WriteArrayStart(TEXT("Customer"));
//
//	for (const auto& Data : Customers)
//	{
//		JsonObj->WriteObjectStart();
//		JsonObj->WriteValue(TEXT("id"), Data.ID);					// ID
//		JsonObj->WriteValue(TEXT("center"), GetCenterName());		// 매장
//		JsonObj->WriteValue(TEXT("loginTime"), Data.LogInTime);		// 로그인 시간
//
//
//		JsonObj->WriteArrayStart(TEXT("model"));							// 고른 제품
//		for (const auto& Idx : Data.PickingModel)
//		{
//			JsonObj->WriteObjectStart();
//			JsonObj->WriteValue(TEXT("code"), Idx.Key);				// 제품 코드
//			JsonObj->WriteValue(TEXT("time"), Idx.Value);					// 일시
//			JsonObj->WriteObjectEnd();
//		}
//		JsonObj->WriteArrayEnd();
//
//		JsonObj->WriteObjectEnd();
//	}
//	JsonObj->WriteArrayEnd();
//	JsonObj->WriteObjectEnd();
//	JsonObj->WriteObjectEnd();
//	JsonObj->Close();
//
//	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
//	FFileHelper::SaveStringToFile(*JsonStr, *Path);
//}

// 아이디, 매장코드, 제품코드, 제품 선택 시간
void UCustomerDataInstance::SendData(const FString Code)
{
	// Json Object 생성
	const FString JsonStr = CreateJsonObject(Code);

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *JsonStr);
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

// 아이디, 매장코드, 로그인 시간
void UCustomerDataInstance::LoginOrSkipCallbackFunction() const
{
	// Json Object 생성
	const FString JsonStr = CreateJsonObject();

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
	UE_LOG(LogTemp, Warning, TEXT("%s"), *JsonStr);
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

FString UCustomerDataInstance::CreateJsonObject(const FString Code) const
{
	// Json Object 생성
	FString JsonStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

	JsonObj->WriteObjectStart();
	JsonObj->WriteObjectStart(TEXT("CustomerInfo"));

	JsonObj->WriteValue(TEXT("id"), CurrentData.ID);					// ID
	JsonObj->WriteValue(TEXT("center"), GetCenterName());			// 매장

	// 레벨 이름 (Start)
	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);

	if(LevelName == "Start")
	{
		JsonObj->WriteValue(TEXT("loginTime"), CurrentData.LogInTime);		// 로그인 시간
	}
	else
	{
		JsonObj->WriteValue(TEXT("code"), Code);							// 제품 코드
		JsonObj->WriteValue(TEXT("selectedTime"), GetCurrentTime());		// 선택일시
	}

	JsonObj->WriteObjectEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->Close();

	return JsonStr;
}

FString UCustomerDataInstance::GetCenterName()
{
	const FString FullPathFromRoot = FPaths::ProjectConfigDir() + "ClientConfig.json";

	FString StringToLoad;
	if (FFileHelper::LoadFileToString(StringToLoad, *FullPathFromRoot))
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

FString UCustomerDataInstance::GetCustomerID(const FString& Str)
{
	FString RString;
	Str.Split("ID\" : \"", nullptr, &RString);
	FString ID;
	RString.Split("\"", &ID, nullptr);

	return ID;
}
