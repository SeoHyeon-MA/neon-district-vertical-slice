// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoCounterWidget.generated.h"

class AShooterCharacter;

/**
 * 탄약 카운터. 폰의 OnBulletCountUpdated 를 구독한다. 폰이 바뀌면 다시 묶는다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UAmmoCounterWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 폰이 바뀔 때마다 부른다. nullptr 이면 구독만 끊고 숨긴다
	void BindToCharacter(AShooterCharacter* Character);
	
protected:
	// 탄약이 이 비율 이하면 경고 색으로
	UPROPERTY(EditDefaultsOnly, Category="Neon District", meta = (ClampMin = 0, ClampMax = 1))
	float LowAmmoRatio = 0.25f;
	
	// 블루프린트가 숫자와 색을 갱신한다
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "Update Ammo"))
	void BP_UpdateAmmo(const FText& CurrentText, const FText& MagazineText, bool bLowAmmo);
	
	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// OnBulletCountUpdated 가 다이나믹 델리게이트라 UFUNCTION 이어야 한다
	UFUNCTION()
	void HandleBulletCountUpdated(int32 MagazineSize, int32 Bullets);
	
	TWeakObjectPtr<AShooterCharacter> BoundCharacter;
};
