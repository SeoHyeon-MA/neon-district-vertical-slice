// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

class ANeonDistrictCharacter;
/**
 *  사망 후 재시작까지 덮는 화면. 새 폰을 받으면 걷는다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 폰이 바뀔 때마다 부른다. nullptr 이면 구독만 끊는다
	void BindToCharacter(ANeonDistrictCharacter* Character);
	
protected:
	// 죽었을 때. 재시작까지 남은 시간을 같이 준다 (연출 길이를 맞출려고)
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta=(DisplayName="On Death"))
	void BP_OnDeath(float RestartDelay);
	
	//~ Begin UUserWidget Interface
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// OnDied 는 일반 멀티캐스트라 UFUNCTION 이 필요 없다
	void HandleDied(ANeonDistrictCharacter* Character);

	TWeakObjectPtr<ANeonDistrictCharacter> BoundCharacter;
};
