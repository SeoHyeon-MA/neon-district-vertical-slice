// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictCharacter.h"
#include "NeonDistrict/InteractionComponent.h"
#include "NeonDistrict/NeonDistrictPlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"
#include "EnhancedInputComponent.h"
#include "NeonDistrictWeapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"


void ANeonDistrictCharacter::UpdateAimPose(float DeltaSeconds)
{
	const ANeonDistrictWeapon* Weapon = Cast<ANeonDistrictWeapon>(CurrentWeapon);
	USkeletalMeshComponent* WeaponMesh = Weapon ? Weapon->GetFirstPersonMesh() : nullptr;
	if (!WeaponMesh) return;
	
	// 팔이 아니라 무기를 옮긴다. 팔은 3인칭 메시의 포즈를 복사해 시점을 따라 기울고,
	// 카메라가 그 팔의 head 본에 타고 있어 총이 화면에 고정된다. 팔을 건드리면 그 구조가 깨진다
	
	// 조준 정도를 우리가 센다. 컴포넌트에서 현재 트랜스폼을 되읽으면
	// 위치와 회전이 각자 수렴해 전환 중에 총신이 호를 그렸다 되돌아온다
	const float TargetAlpha = bIsAiming ? 1.f : 0.f;
	AimAlpha = FMath::FInterpTo(AimAlpha, TargetAlpha, DeltaSeconds, AimBlendSpeed);
	
	// 손 소켓에 SnapToTarget 으로 붙으므로 조준하지 않을 때는 항등이다.
	// 알파 하나가 위치와 회전을 함께 몰아 둘이 어긋날 수 없다
	WeaponMesh->SetRelativeLocation(
		FMath::Lerp(FVector::ZeroVector, Weapon->GetAimOffsetLocation(), AimAlpha));
	
	WeaponMesh->SetRelativeRotation(
		FQuat::Slerp(FQuat::Identity, Weapon->GetAimOffsetRotation().Quaternion(), AimAlpha));
}

void ANeonDistrictCharacter::SetWeaponHolstered(bool bHolstered)
{
	if (!CurrentWeapon) return;
	
	if (bHolstered)
	{
		CurrentWeapon->DeactivateWeapon();
		
		// 템플릿의 OnWeaponDeactivated 는 비어 있어서, 무기만 숨기면
		// 총 없이 사격 자세로 서 있게 된다. 맨손 애님으로 직접 되돌린다
		if (UClass* Unarmed = UnarmedAnimAsset.LoadSynchronous())
		{
			GetMesh()->SetAnimInstanceClass(Unarmed);
		}
	}
	else
	{
		// 애님은 OnWeaponActivated 가 무기에 맞춰 되돌려 준다
		CurrentWeapon->ActivateWeapon(PlayerTag);
	}
}

ANeonDistrictCharacter::ANeonDistrictCharacter()
{
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
}

void ANeonDistrictCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// 조준 해제 시 돌아갈 시야각을 기억한다
	if (UCameraComponent* Camera = GetFirstPersonCameraComponent())
	{
		DefaultFOV = Camera->FieldOfView;
		DefaultFirstPersonFOV = Camera->FirstPersonFieldOfView;
	}
	
}

void ANeonDistrictCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAimPose(DeltaSeconds);
	
	UCameraComponent* Camera = GetFirstPersonCameraComponent();
	if (!Camera || DefaultFOV <= 0.f) return;
	
	const float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;
	if (FMath::IsNearlyEqual(Camera->FieldOfView, TargetFOV, 0.05f)) return;
	
	const float NewFOV = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaSeconds, AimBlendSpeed);
	Camera->SetFieldOfView(NewFOV);
	
	// 1인칭 메시(팔.총)는 별도 시야각으로 그려진다. 월드 FOV 만 바꾸면 둘을 맞추는 보정이
	// 매 프레임 달라져, 카메라에서 먼 부분(총열.사이트)일수록 크게 흔들린다.
	// 비를 유지하면 보정값이 그대로라 총 크기도 유지되고 떨림도 없다
	Camera->FirstPersonFieldOfView = DefaultFirstPersonFOV * (NewFOV / DefaultFOV);
}

void ANeonDistrictCharacter::DoStartAiming()
{
	if (bIsAiming) return;
	
	bIsAiming = true;
	OnAimingChanged.Broadcast(true);
}

void ANeonDistrictCharacter::DoStopAiming()
{
	if (!bIsAiming) return;
	
	bIsAiming = false;
	OnAimingChanged.Broadcast(false);
}

void ANeonDistrictCharacter::Die()
{
	//무기 비활성화, 이동 정지, 충돌 해제, 입력 차단 (부모 기능)
	Super::Die();
	
	DoStopAiming();
	
	SetReloading(false);
	
	OnDied.Broadcast(this);
	
	//부모가 예약한 5초 리스폰을 취소
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	
	//대신 잠시 뒤 레벨을 다시 시작
	GetWorldTimerManager().SetTimer(RestartTimer, this, &ANeonDistrictCharacter::RequestRestart, RestartDelay, false);
}

void ANeonDistrictCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (InteractAction)
		{
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ANeonDistrictCharacter::DoInteract);
		}
		if (AimAction)
		{
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &ANeonDistrictCharacter::DoStartAiming);
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &ANeonDistrictCharacter::DoStopAiming);
		}
		if (ReloadAction)
		{
			EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANeonDistrictCharacter::DoReload);
		}
	}
}

void ANeonDistrictCharacter::DoReload()
{
	if (ANeonDistrictWeapon* Weapon = Cast<ANeonDistrictWeapon>(CurrentWeapon))
	{
		Weapon->StartReload();
	}
}

void ANeonDistrictCharacter::DoInteract()
{
	// 대화 중이면 E는 "다음 줄 실행"
	if (ANeonDistrictPlayerController* PC = GetController<ANeonDistrictPlayerController>())
	{
		if (PC->IsInDialogue())
		{
			PC->AdvanceDialogue();
			return;
		}
	}
	
	InteractionComponent->TryInteract();
}

void ANeonDistrictCharacter::RequestRestart()
{
	// Destroy() 뒤에는 GetController()가 nullptr이므로 미리 잡아둔다
	AController* MyController = GetController();
	UWorld* World = GetWorld();
	
	//캐릭터만 파괴한다. 구간 리셋은 미션 시스템이 생기면 게임모드가 맡는다.
	Destroy();
	
	if (World && MyController)
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			GameMode->RestartPlayer(MyController);
		}
	}
}

void ANeonDistrictCharacter::NDKill()
{
	TakeDamage(MaxHP * 2.0f, FDamageEvent(), nullptr, this);
}

void ANeonDistrictCharacter::SetReloading(bool bNewReloading)
{
	if (bIsReloading == bNewReloading) return;
	
	bIsReloading = bNewReloading;
	OnReloadingChanged.Broadcast(bIsReloading);
}