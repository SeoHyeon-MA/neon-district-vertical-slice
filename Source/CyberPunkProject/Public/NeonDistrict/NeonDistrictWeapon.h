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
	
	//총 모델
	UPROPERTY(EditDefaultsOnly, Category="NeonDistict|Mesh")
	TSoftObjectPtr<UStaticMesh> GunMeshAsset;
	
	//손 소켓 기준
	//위치
	UPROPERTY(EditDefaultsOnly, Category="NeonDistict|Mesh")
	FVector GunMeshLocation = FVector::ZeroVector;
	
	//회전
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict|Mesh")
	FRotator GunMeshRotation = FRotator::ZeroRotator;
	
	//균일 배율
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict|Mesh")
	float GunMeshScale = 1.f;
	
	//재장전에 걸리는 시간
	UPROPERTY(EditDefaultsOnly, Category= "NeonDistrict|Reload", meta = (ClampMin = 0.1, ClampMax = 10, Units = "s"))
	float ReloadDuration = 1.5f;
	
	bool bIsReloading = false;
	FTimerHandle ReloadTimer;
	
public:
	bool IsReloading() const { return bIsReloading; }
	
	// 탄창이 가득 찼거나 이미 재장전 중이면 무시한다
	void StartReload();
	
protected:
	//~ Begin AShooterWeapon Interface
	virtual void Fire() override;
	virtual void FireProjectile(const FVector& TargetLocation) override;
	//~ End AShooterWeapon Interface
	
	void FinishReload();
};
