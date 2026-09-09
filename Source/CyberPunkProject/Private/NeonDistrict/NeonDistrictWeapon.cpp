// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
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

}

void ANeonDistrictWeapon::BeginPlay()
{
	// 총 모델
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Fab/Sci-fi_Gun_Venra-46_/sci_fi_gunvenra_46/StaticMeshes/sci_fi_gunvenra_46.sci_fi_gunvenra_46")))
	{
		GunMesh->SetStaticMesh(Mesh);
		
		// 총 크기 조절
		GunMesh->SetRelativeScale3D(FVector(0.3f));
		// 총 위치 조절
		GunMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		// 총 방향 조절
		GunMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	}

	// 발사할 투사체
	if (UClass* Projectile = LoadClass<AShooterProjectile>(nullptr,
		TEXT("/Game/Variant_Shooter/Blueprints/Pickups/Projectiles/BP_ShooterProjectile_Bullet.BP_ShooterProjectile_Bullet_C")))
	{
		ProjectileClass = Projectile;
	}

	// 이 총을 들었을 때 팔과 몸이 쓸 애니메이션
	if (UClass* FirstPersonABP = FirstPersonAnimAsset.LoadSynchronous())
	{
		FirstPersonAnimInstanceClass = FirstPersonABP;
	}

	if (UClass* ThirdPersonABP = ThirdPersonAnimAsset.LoadSynchronous())
	{
		ThirdPersonAnimInstanceClass = ThirdPersonABP;
	}

	Super::BeginPlay();
}
