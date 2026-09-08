// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Variant_Shooter/Weapons/ShooterProjectile.h"

ANeonDistrictWeapon::ANeonDistrictWeapon()
{
	//스태틱 메시 컴포넌트 생성
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	
	//1인칭 메시의 자식으로 붙인다
	GunMesh->SetupAttachment(GetFirstPersonMesh());
	
	//총알이 자기 총에 맞지 않도록
	GunMesh->SetCollisionProfileName(FName("NoCollision"));
	
	//1인칭 전용 렌더링
	GunMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	GunMesh->bOnlyOwnerSee = true;

	//총 모델 에셋을 찾아서 넣는다
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GunMeshAsset(TEXT("/Game/Fab/Sci-fi_Gun_Venra-46_/sci_fi_gunvenra_46/StaticMeshes/sci_fi_gunvenra_46.sci_fi_gunvenra_46"));
	
	if (GunMeshAsset.Succeeded())
	{
		GunMesh->SetStaticMesh(GunMeshAsset.Object);
	}
	
	// 발사할 투사체 클래스 지정
	static ConstructorHelpers::FClassFinder<AShooterProjectile> ProjectileBP(
		TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/BP_ShooterProjectile_Bullet"));

	if (ProjectileBP.Succeeded())
	{
		ProjectileClass = ProjectileBP.Class;
	}
}
