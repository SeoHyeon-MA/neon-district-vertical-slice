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
	
	//스태틱 메시로 된 총 모델. 1인칭 전용이라 소유 플레이어의 1인칭 화면에만 그려진다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* GunMesh;
	
	//같은 총 모델의 3인칭용. 대화 카메라.시네마틱.적이 보는 것은 이쪽이다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* GunMeshThirdPerson;
	
public:
	ANeonDistrictWeapon();

protected: 

	/** 에셋 로드는 생성자가 아니라 여기서 한다 */
	virtual void BeginPlay() override;
	
	/** 메시와 트랜스폼은 여기서 적용한다. 값을 바꾸면 에디터.PIE 에서 바로 반영된다 */
	virtual void OnConstruction(const FTransform& Transform) override;
	
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
	
	//1인칭 손 소켓 기준
	//위치
	UPROPERTY(EditDefaultsOnly, Category="NeonDistict|Mesh")
	FVector GunMeshLocation = FVector::ZeroVector;
	
	//회전
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict|Mesh")
	FRotator GunMeshRotation = FRotator::ZeroRotator;
	
	//균일 배율
	UPROPERTY(EditDefaultsOnly, Category = "NeonDistict|Mesh")
	float GunMeshScale = 1.f;
	
	//3인칭 손 소켓 기준. 1인칭과 소켓 위치가 달라 값을 따로 둔다.
	//EditAnywhere 라서 PIE 중 스폰된 무기에서 바로 조절할 수 있다 (찾은 값은 BP 기본값에 옮긴다)
	UPROPERTY(EditAnywhere, Category="NeonDistict|Mesh (3인칭)")
	FVector GunMeshLocationThirdPerson = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "NeonDistict|Mesh (3인칭)")
	FRotator GunMeshRotationThirdPerson = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, Category = "NeonDistict|Mesh (3인칭)")
	float GunMeshScaleThirdPerson = 1.f;
	
	// 이 무기로 조준할 때 팔 메시를 얼마나 옮길지. 에디터에서 눈으로 맞춘다
	UPROPERTY(EditAnywhere, Category="NeonDistrict|Aim")
	FVector AimOffsetLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, Category = "NeonDistrict|Aim")
	FRotator AimOffsetRotation = FRotator::ZeroRotator;
	
	//재장전에 걸리는 시간
	UPROPERTY(EditDefaultsOnly, Category= "NeonDistrict|Reload", meta = (ClampMin = 0.1, ClampMax = 10, Units = "s"))
	float ReloadDuration = 1.5f;
	
	bool bIsReloading = false;
	FTimerHandle ReloadTimer;
	
public:
	bool IsReloading() const { return bIsReloading; }
	
	// 탄창이 가득 찼거나 이미 재장전 중이면 무시한다
	void StartReload();
	
	const FVector& GetAimOffsetLocation() const { return AimOffsetLocation; }
	const FRotator& GetAimOffsetRotation() const { return AimOffsetRotation; }
	
protected:
	//~ Begin AShooterWeapon Interface
	virtual void Fire() override;
	virtual void FireProjectile(const FVector& TargetLocation) override;
	//~ End AShooterWeapon Interface
	
	void FinishReload();
};
