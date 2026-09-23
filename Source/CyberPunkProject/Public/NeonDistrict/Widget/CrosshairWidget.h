// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class ANeonDistrictCharacter;
/**
 *  조준점. 캐릭터의 OnAimingChanged 를 구독해 조준 중이면 모양을 바꾼다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 폰이 바뀔 때마다 부른다. nullptr 이면 구독만 끊고 숨긴다
	void BindToCharacter (ANeonDistrictCharacter* Character);

protected:
	// 블루프린트가 모양을 바꾼다
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "Set Aiming"))
	void BP_SetAiming(bool bAiming);
	
	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	void HandleAimingChanged(bool bAiming);
	
	TWeakObjectPtr<ANeonDistrictCharacter> BoundCharacter;
};
