// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HTTPServer/Public/IHttpRouter.h"
#include "Samsung_QR/DataObject/DataObject.h"
#include "QR_Widget.generated.h"

class UImage;
class IHttpRouter;
class FHttpServerModule;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReceiveComplete);


/**
 * 
 */
UCLASS()
class SAMSUNG_QR_API UQR_Widget : public UUserWidget
{
	GENERATED_BODY()

public:

	FHttpServerModule* Server;
	TSharedPtr<IHttpRouter> Router;
	FHttpRouteHandle RouteHandle;
	bool bClosed;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void StartServer();
	void StopServer();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Http")
	FString HttpPath = TEXT("/qr");

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Http")
	int32 ServerPort = 8080;

	UFUNCTION(BlueprintCallable)
	bool CustomerJsonFileCheck();

	FString GetCustomerFileName();

	FString GetCenterName();

	FString GetCustomerID(const FString& Str);

	FString GetCurrentTime();

	// 누가 어느매장에서 / 어떤제품을 언제 눌렀다.

	UFUNCTION(BlueprintCallable)
	void OnClickedSkipBtn();

	UFUNCTION(BlueprintCallable)
	void OnClickedExitBtn();


private:

	// Response Delegate
	UPROPERTY(BlueprintAssignable)
	FOnReceiveComplete OnReceiveComplete;

public:
	// QR 이미지
	UPROPERTY(EditAnywhere, meta = (BindWidget), BlueprintReadWrite)
	UImage* QR_Image;

	// Load QR Image ../Plugins/Samsung_QR/Content/QR_code.png
	UFUNCTION(BlueprintCallable)
	UTexture2D* LoadQRTexture_FromFile();

	// 고객 데이터를 담는 액터
	UPROPERTY()
	ADataActor* DataActor;

};
	
