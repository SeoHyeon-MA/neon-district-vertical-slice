// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictRifle.h"

ANeonDistrictRifle::ANeonDistrictRifle()
{
	// 두 손 소총 자세
	FirstPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Weapon.ABP_FP_Weapon_C")));

	ThirdPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Rifle.ABP_TP_Rifle_C")));
	
	// 소총 설정
	bFullAuto = true;
	RefireRate = 0.12f;
	MagazineSize = 30;
}
