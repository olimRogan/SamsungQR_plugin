// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CustomerDataInstance.generated.h"

DECLARE_DELEGATE(FLoginOrSkipDelegate);


USTRUCT(Atomic, BlueprintType)
struct FCustomer
{
	GENERATED_BODY()

	// 매장 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Center;

	// 고객 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ID;

	// 고른 상품들 (상품코드 / 누른 시간)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString,FString> PickingModel;

	// 로그인 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LogInTime;
};

// 
UENUM(BlueprintType)
enum class ESuccessType : uint8
{
	EST_Login UMETA(DisplayName = "Login"),
	EST_Skip UMETA(DisplayName = "Skip"),
};

/**
 * 
 */
UCLASS()
class SAMSUNG_QR_API UCustomerDataInstance : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 로그인 또는 건너뛰기 콜백 델리게이트
	FLoginOrSkipDelegate LoginOrSkipCallback;

	// 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 고객 데이터 배열 -> 제품 하나씩 데이터를 보낼 경우 필요 없을 듯
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<FCustomer> CustomerDataArr;

	// 현재 고객 데이터
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FCustomer CurrentData;

	// json 형태로 db 저장
	//UFUNCTION(BlueprintCallable)
	//void CreateDB(FCustomer Customer,TArray<FCustomer> Customers);

	// 우선은 함수 이름을 SendData 로 저정 (현재는 json 형태로 저장)
	UFUNCTION(BlueprintCallable)
	void SendData(const FString Code);

	// 로그인 또는 건너뛰기 콜백 함수
	UFUNCTION()
	void LoginOrSkipCallbackFunction() const;

	// Json (아이디, 매장코드, 제품코드, 일시)
	FString CreateJsonObject(const FString Code = "") const;

	// 현재 시간 받아오기 (년, 월, 일, 시, 분, 초)
	UFUNCTION(BlueprintCallable)
	static FString GetCurrentTime();

	// 현지 시간 받아오기 (년, 월, 일)
	static FString GetCurrentYearMonthDay();

	// 매장 이름 받아오기 (ClientConfig.json)
	static FString GetCenterName();

	// 고객 ID 받아오기
	static FString GetCustomerID(const FString& Str);
};
