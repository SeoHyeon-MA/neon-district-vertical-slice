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
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingChanged, bool /*bAiming*/);

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
	
	UPROPERTY(EditAnywhere, Category="Neon District|Aim")
	UInputAction* AimAction;
	
	// 조준했을 때 시야각. 좁을수록 확대돼 보인다
	UPROPERTY(EditAnywhere, Category="Neon District|Aim", meta = (ClampMin = 30, ClampMax = 120, Units = "Degrees"))
	float AimFOV = 55.f;
	
	// 시야각 보간 속도. 클수록 빠르다
	UPROPERTY(EditAnywhere, Category="Neon District|Aim", meta = (ClampMin = 1, ClampMax =30))
	float AimBlendSpeed = 12.f;
	
	//죽고 나서 재시작까지의 시간
	UPROPERTY(EditDefaultsOnly, Category="NeonDistrict", meta=(ClampMin=0, ClampMax=10))
	float RestartDelay = 2.f;
	
	FTimerHandle RestartTimer;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District|Aim")
	bool bIsAiming = false;
	
	// BeginPlay 에서 카메라의 원래 시야각을 기억한다.
	float DefaultFOV = 0.f;
	
public:
	ANeonDistrictCharacter();
	
protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	//~ End AActor Interface
	
	//리스폰 대신 재시작
	virtual void Die() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void DoInteract();
	
	void DoStartAiming();
	void DoStopAiming();
	
	//레벨을 처음부터 다시 연다
	void RequestRestart();
	
	UFUNCTION(Exec)
	void NDKill();

public:
	// 죽을 때 방송된다. 미션이 구독한다.
	FOnNeonCharacterDied OnDied;
	
	//조준 상태가 바뀔 때 방송. 크로스헤어가 구독한다.
	FOnAimingChanged OnAimingChanged;
	
	bool IsAiming() const { return bIsAiming; }
};
