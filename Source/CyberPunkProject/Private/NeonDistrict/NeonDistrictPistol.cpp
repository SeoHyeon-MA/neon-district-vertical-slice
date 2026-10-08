// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictPistol.h"

ANeonDistrictPistol::ANeonDistrictPistol()
{
	// 한 손 권총 자세
	FirstPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Pistol.ABP_FP_Pistol_C")));

	ThirdPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C")));

	GunMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Weapons/Pistol/Meshes/SM_Pistol.SM_Pistol")));
	
	GunMeshScale = 1.f;
	GunMeshLocation = FVector::ZeroVector;
	GunMeshRotation = FRotator::ZeroRotator;
	
	// 3인칭도 같은 메시라 배율은 같다. 손 소켓 위치가 달라 자리만 따로 맞춘다
	GunMeshScaleThirdPerson = GunMeshScale;
}
