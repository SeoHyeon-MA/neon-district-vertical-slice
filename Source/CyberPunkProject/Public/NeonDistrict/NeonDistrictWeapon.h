// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/Weapons/ShooterWeapon.h"
#include "NeonDistrictWeapon.generated.h"

/**
 * 
 */
UCLASS(abstract)
class CYBERPUNKPROJECT_API ANeonDistrictWeapon : public AShooterWeapon
{
	GENERATED_BODY()
	
	//스태틱 메시로 된 총 모델
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* GunMesh;
	
public:
	ANeonDistrictWeapon();

protected: 

	/** 에셋 로드는 생성자가 아니라 여기서 한다 */
	virtual void BeginPlay() override;
	
protected:
	//이 총을 들었을때 1인칭 팔이 쓸 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict")
	TSoftClassPtr<UAnimInstance> FirstPersonAnimAsset;
	
	//3인칭 몸이 쓸 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict")
	TSoftClassPtr<UAnimInstance> ThirdPersonAnimAsset;
};
