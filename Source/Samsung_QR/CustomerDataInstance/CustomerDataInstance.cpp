// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomerDataInstance.h"

#include "Kismet/KismetMathLibrary.h"

void UCustomerDataInstance::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// ��������Ʈ ���ε�
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


	return FString::Printf(TEXT("%d_%d_%d_%02d_%02d_%02d"), Year, Month, Day, Hour, Minute, Second);
}

FString UCustomerDataInstance::GetCurrentYearMonthDay()
{
	// ���� ��, ��, �� �޾ƿ���
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();

	return FString::Printf(TEXT("%d_%d_%d.json"), Year, Month, Day);
}

void UCustomerDataInstance::CreateDB(FCustomer Customer,TArray<FCustomer> Customers)
{
	// Json Object ����
	FString JsonStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

	// Arr �� �߰� �� �ʱ�ȭ
	Customers.Emplace(Customer);
	Customer = FCustomer();

	JsonObj->WriteObjectStart();
	JsonObj->WriteObjectStart(TEXT("CustomerInfo"));
	JsonObj->WriteArrayStart(TEXT("Customer"));

	for (const auto& Data : Customers)
	{
		JsonObj->WriteObjectStart();
		JsonObj->WriteValue(TEXT("id"), Data.ID);					// ID
		JsonObj->WriteValue(TEXT("center"), GetCenterName());		// ����
		JsonObj->WriteValue(TEXT("loginTime"), Data.LogInTime);		// �α��� �ð�
		

		JsonObj->WriteArrayStart(TEXT("model"));							// �� ��ǰ
		for (const auto& Idx : Data.PickingModel)
		{
			JsonObj->WriteObjectStart();
			JsonObj->WriteValue(TEXT("code"), Idx.Key);				// ��ǰ �ڵ�
			JsonObj->WriteValue(TEXT("time"), Idx.Value);					// �Ͻ�
			JsonObj->WriteObjectEnd();
		}
		JsonObj->WriteArrayEnd();

		JsonObj->WriteObjectEnd();
	}
	JsonObj->WriteArrayEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->Close();

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

void UCustomerDataInstance::SendData(const FString Code)
{
	// Json Object ����
	const FString JsonStr = CreateJsonObjectWithCode(Code);

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

void UCustomerDataInstance::LoginOrSkipCallbackFunction()
{
	// Json Object ����
	const FString JsonStr = CreateJsonObject();

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + GetCurrentYearMonthDay();
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

FString UCustomerDataInstance::CreateJsonObject() const
{
	// Json Object ����
	FString JsonStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

	JsonObj->WriteObjectStart();
	JsonObj->WriteObjectStart(TEXT("CustomerInfo"));

	JsonObj->WriteValue(TEXT("id"), CurrentData.ID);					// ID
	JsonObj->WriteValue(TEXT("center"), GetCenterName());			// ����
	JsonObj->WriteValue(TEXT("loginTime"), CurrentData.LogInTime);		// �α��� �ð�

	JsonObj->WriteObjectEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->Close();

	return JsonStr;
}

FString UCustomerDataInstance::CreateJsonObjectWithCode(const FString Code) const
{
	// Json Object ����
	FString JsonStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonObj = TJsonWriterFactory<>::Create(&JsonStr);

	JsonObj->WriteObjectStart();
	JsonObj->WriteObjectStart(TEXT("CustomerInfo"));

	JsonObj->WriteValue(TEXT("id"), CurrentData.ID);					// ID
	JsonObj->WriteValue(TEXT("center"), GetCenterName());			// ����
	JsonObj->WriteValue(TEXT("code"), Code);							// ��ǰ �ڵ�
	JsonObj->WriteValue(TEXT("selectedTime"), GetCurrentTime());		// �����Ͻ�

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
