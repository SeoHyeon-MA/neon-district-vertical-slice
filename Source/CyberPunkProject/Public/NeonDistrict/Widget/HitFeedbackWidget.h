// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HitFeedbackWidget.generated.h"

class AShooterCharacter;

/**
 *  피격 시 화면 가장자리를 붉게. 체력이 줄었을 때만 반응한다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UHitFeedbackWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 폰이 바뀔 때마다 부른다. nullptr 이면 구독만 끊는다.
	void BindToCharacter(AShooterCharacter* Character);
	
protected:
	// 맞았을 때. 남은 체력 비율을 같이 준다 (낮을수록 진하게 쓸 수 있다)
	UFUNCTION(BlueprintImplementableEvent, Category= "Neon District", meta = (DisplayName = "On Hit"))
	void BP_OnHit(float LifePercent);
	
	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// OnDamaged가 다이나믹 델리게이트라 UFUNCTION 이어야 한다
	UFUNCTION()
	void HandleDamaged(float LifePercent);
	
	TWeakObjectPtr<AShooterCharacter> BoundCharacter;
	
	// 줄었는지 판단하려고 직전 값을 기억한다
	float LastLiftPercent = 1.f;
};
