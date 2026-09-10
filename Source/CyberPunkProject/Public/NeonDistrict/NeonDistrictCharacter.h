// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterCharacter.h"
#include "NeonDistrictCharacter.generated.h"

/**
 * 
 */
UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictCharacter : public AShooterCharacter
{
	GENERATED_BODY()
	
protected:
	//죽고 나서 재시작까지의 시간
	UPROPERTY(EditDefaultsOnly, Category="NeonDistrict", meta=(ClampMin=0, ClampMax=10))
	float RestartDelay = 2.f;
	
	FTimerHandle RestartTimer;
	
	//리스폰 대신 재시작
	virtual void Die() override;
	
	//레벨을 처음부터 다시 연다
	void RequestRestart();
	
	UFUNCTION(Exec)
	void NDKill();
};
