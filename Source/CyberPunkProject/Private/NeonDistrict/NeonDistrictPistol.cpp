// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictPistol.h"

ANeonDistrictPistol::ANeonDistrictPistol()
{
	// 한 손 권총 자세
	FirstPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_FP_Pistol.ABP_FP_Pistol_C")));

	ThirdPersonAnimAsset = TSoftClassPtr<UAnimInstance>(FSoftObjectPath(TEXT("/Game/Variant_Shooter/Anims/ABP_TP_Pistol.ABP_TP_Pistol_C")));
}
