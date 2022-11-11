// Fill out your copyright notice in the Description page of Project Settings.


#include "DataObject.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADataActor::ADataActor()
{
	PrimaryActorTick.bCanEverTick = true;

	TimeoutCount = 5.f;
}

void ADataActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

}

FString ADataActor::GetCurrentTime()
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

void ADataActor::CreateDB(FCustomer Customer,TArray<FCustomer> Customers)
{
	// ���� ��, ��, �� �޾ƿ���
	const FDateTime UTCTime = UKismetMathLibrary::UtcNow();

	const int32 Year = UTCTime.GetYear();
	const int32 Month = UTCTime.GetMonth();
	const int32 Day = UTCTime.GetDay();

	const FString FileName = FString::Printf(TEXT("%d_%d_%d.json"), Year, Month, Day);

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
		JsonObj->WriteValue(TEXT("loginTime"), Data.LogInTime);	// �α��� �ð�
		

		JsonObj->WriteArrayStart(TEXT("model"));						// �� ��ǰ
		for (const auto& Idx : Data.PickingModel)
		{
			JsonObj->WriteObjectStart();
			JsonObj->WriteValue(TEXT("code"), Idx.Key);			// ��ǰ �ڵ�
			JsonObj->WriteValue(TEXT("time"), Idx.Value);				// �Ͻ�
			JsonObj->WriteObjectEnd();
		}
		JsonObj->WriteArrayEnd();

		JsonObj->WriteObjectEnd();
	}
	JsonObj->WriteArrayEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->WriteObjectEnd();
	JsonObj->Close();

	const FString Path = FPaths::ProjectPluginsDir() + "Samsung_QR/Content/DB/" + FileName;
	FFileHelper::SaveStringToFile(*JsonStr, *Path);
}

FString ADataActor::GetCenterName()
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

void ADataActor::Timeout()
{
	FTimerHandle T;
	GetWorldTimerManager().SetTimer(T, [this]
		{
			UGameplayStatics::OpenLevel(this, TEXT("Start"));
		},TimeoutCount,false);
}
