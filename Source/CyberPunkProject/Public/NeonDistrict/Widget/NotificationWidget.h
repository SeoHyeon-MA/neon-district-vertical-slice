// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotificationWidget.generated.h"

class UNotificationSubsystem;
/**
 *  알림 통로에서 오는 한 줄을 짧게 띄운다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UNotificationWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	// 블루프린트가 문장을 채우고 연출을 재생한다
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName="ShowNotification"))
	void BP_ShowNotification(const FText& Message);
	
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	void HandleNotification(const FText& Message);
	
	TWeakObjectPtr<UNotificationSubsystem> BoundSubsystem;
};
