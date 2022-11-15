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

	// ���� ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Center;

	// �� ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString ID;

	// �� ��ǰ�� (��ǰ�ڵ� / ���� �ð�)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FString,FString> PickingModel;

	// �α��� �ð�
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
	// �α��� �Ǵ� �ǳʶٱ� �ݹ� ��������Ʈ
	FLoginOrSkipDelegate LoginOrSkipCallback;

	// �ʱ�ȭ
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// �� ������ �迭 -> ��ǰ �ϳ��� �����͸� ���� ��� �ʿ� ���� ��
	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TArray<FCustomer> CustomerDataArr;

	// ���� �� ������
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FCustomer CurrentData;

	// json ���·� db ����
	//UFUNCTION(BlueprintCallable)
	//void CreateDB(FCustomer Customer,TArray<FCustomer> Customers);

	// �켱�� �Լ� �̸��� SendData �� ���� (����� json ���·� ����)
	UFUNCTION(BlueprintCallable)
	void SendData(const FString Code);

	// �α��� �Ǵ� �ǳʶٱ� �ݹ� �Լ�
	UFUNCTION()
	void LoginOrSkipCallbackFunction() const;

	// Json (���̵�, �����ڵ�, ��ǰ�ڵ�, �Ͻ�)
	FString CreateJsonObject(const FString Code = "") const;

	// ���� �ð� �޾ƿ��� (��, ��, ��, ��, ��, ��)
	UFUNCTION(BlueprintCallable)
	static FString GetCurrentTime();

	// ���� �ð� �޾ƿ��� (��, ��, ��)
	static FString GetCurrentYearMonthDay();

	// ���� �̸� �޾ƿ��� (ClientConfig.json)
	static FString GetCenterName();

	// �� ID �޾ƿ���
	static FString GetCustomerID(const FString& Str);
};
