// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class AShooterCharacter;
/**
 *  체력 바. 폰의 OnDamaged를 구독한다. 폰이 바뀌면 다시 묶는다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 폰이 바뀔 때마다 부른다. nullptr 이면 구독만 끊는다
	void BindToCharacter(AShooterCharacter* Character);
	
protected:
	// 블루프린트가 프로그레스 바를 갱신. 0~1
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta=(DisplayName="Update Health Bar"))
	void BP_UpdateHealthBar(float LifePercent);

	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// OnDamaged 가 다이나믹 델리게이트라 UFUNTION 이어야 한다
	UFUNCTION()
	void HandleDamaged(float LifePercent);
	
	TWeakObjectPtr<AShooterCharacter> BoundCharacter;
};
