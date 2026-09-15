// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "NeonDistrictCharacter.generated.h"

/**
 * 
 */

class ANeonDistrictCharacter;
class UInteractionComponent;
class UInputAction;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNeonCharacterDied, ANeonDistrictCharacter*)

UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictCharacter : public AShooterCharacter
{
	GENERATED_BODY()
	
protected:
	// 앞을 훑어 상호작용 대상을 찾는다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UInteractionComponent* InteractionComponent;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* InteractAction;
	
	//죽고 나서 재시작까지의 시간
	UPROPERTY(EditDefaultsOnly, Category="NeonDistrict", meta=(ClampMin=0, ClampMax=10))
	float RestartDelay = 2.f;
	
	FTimerHandle RestartTimer;
	
public:
	ANeonDistrictCharacter();
	
protected:
	//리스폰 대신 재시작
	virtual void Die() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void DoInteract();
	
	//레벨을 처음부터 다시 연다
	void RequestRestart();
	
	UFUNCTION(Exec)
	void NDKill();

public:
	// 죽을 때 방송된다. 미션이 구독한다.
	FOnNeonCharacterDied OnDied;
};
