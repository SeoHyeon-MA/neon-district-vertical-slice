// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictWeapon.h"
#include "NeonDistrict/NeonDistrictCharacter.h"
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
	
	//3인칭용 총. 대화 카메라와 적이 보는 것은 이쪽이다.
	//템플릿이 ThirdPersonMesh 를 3인칭 손 소켓에 붙여 주므로 그 자식으로 둔다
	GunMeshThirdPerson = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh Third Person"));
	GunMeshThirdPerson->SetupAttachment(GetThirdPersonMesh());
	
	GunMeshThirdPerson->SetCollisionProfileName(FName("NoCollision"));
	
	//소유자에게는 1인칭 총이 따로 있으므로 3인칭 총은 가린다. 남이 보는 화면에는 이것만 보인다
	GunMeshThirdPerson->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	GunMeshThirdPerson->bOwnerNoSee = true;
}

void ANeonDistrictWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// 총 모델. 1인칭.3인칭이 같은 메시를 쓴다
	if (UStaticMesh* Mesh = GunMeshAsset.LoadSynchronous())
	{
		GunMesh->SetStaticMesh(Mesh);
		GunMeshThirdPerson->SetStaticMesh(Mesh);
	}
	
	GunMesh->SetRelativeScale3D(FVector(GunMeshScale));
	GunMesh->SetRelativeLocation(GunMeshLocation);
	GunMesh->SetRelativeRotation(GunMeshRotation);
	
	// 3인칭 손 소켓은 자리가 달라 값을 따로 받는다
	GunMeshThirdPerson->SetRelativeScale3D(FVector(GunMeshScaleThirdPerson));
	GunMeshThirdPerson->SetRelativeLocation(GunMeshLocationThirdPerson);
	GunMeshThirdPerson->SetRelativeRotation(GunMeshRotationThirdPerson);
}

void ANeonDistrictWeapon::BeginPlay()
{

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

void ANeonDistrictWeapon::Fire()
{
	if (bIsReloading)
	{
		return;
	}
	
	// 탄창이 비었으면 발사를 멈춘다. 재장전은 R 로만
	if (CurrentBullets <= 0)
	{
		StopFiring();
		return;
	}
	
	Super::Fire();
}

void ANeonDistrictWeapon::FireProjectile(const FVector& TargetLocation)
{
	const int32 BulletBefore = CurrentBullets;
	
	Super::FireProjectile(TargetLocation);
	
	// 템플릿-마지막 탄을 쏘면 바로 탄창을 채움 / 수정-비운채로 둠
	if (BulletBefore <= 1)
	{
		CurrentBullets = 0;
		
		if (WeaponOwner)
		{
			WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
		}
	}
}

void ANeonDistrictWeapon::StartReload()
{
	if (bIsReloading || CurrentBullets >= MagazineSize)
	{
		return;
	}
	
	bIsReloading = true;
	
	//방아쇠를 누르고 있어도 멈춘다
	StopFiring();
	
	GetWorldTimerManager().SetTimer(ReloadTimer, this, &ANeonDistrictWeapon::FinishReload, ReloadDuration, false);
	
	if (ANeonDistrictCharacter* Character = Cast<ANeonDistrictCharacter>(PawnOwner))
	{
		Character->SetReloading(true);
	}
}

void ANeonDistrictWeapon::FinishReload()
{
	bIsReloading = false;
	CurrentBullets = MagazineSize;
	
	if (WeaponOwner)
	{
		WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
	}
	
	if (ANeonDistrictCharacter* Character = Cast<ANeonDistrictCharacter>(PawnOwner))
	{
		Character->SetReloading(false);
	}
}
