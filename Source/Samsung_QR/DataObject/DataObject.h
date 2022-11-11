// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataObject.generated.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString LogInTime;
};

/**
 * 
 */
UCLASS()
class SAMSUNG_QR_API ADataActor : public AActor
{
	GENERATED_BODY()

public:

	ADataActor();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<FCustomer> CustomerDataArr;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FCustomer CurrentData;

	UFUNCTION(BlueprintCallable)
	FString GetCurrentTime();

	UFUNCTION(BlueprintCallable)
	void CreateDB(FCustomer Customer,TArray<FCustomer> Customers);

	FString GetCenterName();

	void Timeout();

	UPROPERTY(BlueprintReadWrite)
	float TimeoutCount;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMove;
};
