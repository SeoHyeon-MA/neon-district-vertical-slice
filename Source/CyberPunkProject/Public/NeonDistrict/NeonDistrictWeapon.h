// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "NeonDistrictWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictWeapon : public AShooterWeapon
{
	GENERATED_BODY()
	
	//스태틱 메시로 된 총 모델
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* GunMesh;
	
public:
	ANeonDistrictWeapon();
	
};
