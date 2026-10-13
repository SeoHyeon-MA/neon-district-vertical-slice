// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictRifle.h"

ANeonDistrictRifle::ANeonDistrictRifle()
{
	// 두 손 소총 자세
	FirstPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Weapon.ABP_FP_Weapon_C")));

	ThirdPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C")));
	
	// 기본 투사체. 무기 BP 에서 덮어쓸 수 있다
	ProjectileAsset = TSoftClassPtr<AShooterProjectile>(FSoftObjectPath(TEXT("/Game/NeonDistrict/Blueprints/BP_NeonDistrictBullet.BP_NeonDistrictBullet_C")));
	
	// 소총 설정
	bFullAuto = true;
	RefireRate = 0.12f;
	MagazineSize = 30;
	
	//스태틱 메쉬 설정
	GunMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Fab/Sci-fi_Gun_Venra-46_/sci_fi_gunvenra_46/StaticMeshes/sci_fi_gunvenra_46.sci_fi_gunvenra_46")));
	
	// Fab 총 원본은 길이 401cm 다. 템플릿 SM_Rifle(78.6cm)에 맞춘 배율 = 78.6 / 401.0
	// 눈으로 맞춘 값이 아니라 마네킹 손에 맞는 크기가 검증된 메시에서 역산한 값이다
	GunMeshScale = 0.196f;
	GunMeshLocation = FVector(0.f, 0.f, 8.f);
	GunMeshRotation = FRotator::ZeroRotator;
	
	// 1인칭과 3인칭이 월드에서 같은 크기다
	GunMeshScaleThirdPerson = GunMeshScale;
	
	// 눈으로 맞춘 값. 에디터 Details 의 Rotation 표시는 X=Roll, Y=Pitch, Z=Yaw 순이다
	//   Location (0.0, 13.2, 5.0)   Rotation X 9.5  Y -2.8  Z -1.8
	GunMeshLocationThirdPerson = FVector(0.f, 13.2f, 5.f);
	GunMeshRotationThirdPerson = FRotator(-2.8f, -1.8f, 9.5f);   // FRotator(Pitch, Yaw, Roll)
}
