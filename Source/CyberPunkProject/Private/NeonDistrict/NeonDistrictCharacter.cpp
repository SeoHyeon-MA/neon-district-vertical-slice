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

	// 손 소켓에 SnapToTarget 으로 붙으므로 평소 상대 트랜스폼은 항등이다
	FVector TargetLocation = FVector::ZeroVector;
	FQuat TargetQuat = FQuat::Identity;

	if (bIsAiming)
	{
		// 조준 위치는 무기마다 다르다. 무기에게 묻는다
		TargetLocation = Weapon->GetAimOffsetLocation();

		// FRotator 덧셈은 Pitch/Yaw/Roll 숫자를 각각 더할 뿐이라 축이 섞인다.
		// 회전은 쿼터니언으로 다룬다
		TargetQuat = Weapon->GetAimOffsetRotation().Quaternion();
	}

	// 시야각과 같은 속도로 움직여야 둘이 한 동작으로 보인다
	const float Alpha = FMath::Clamp(DeltaSeconds * AimBlendSpeed, 0.f, 1.f);

	WeaponMesh->SetRelativeLocation(FMath::VInterpTo(WeaponMesh->GetRelativeLocation(), TargetLocation, DeltaSeconds, AimBlendSpeed));

	// 쿼터니언 보간. FRotator 보간은 짐벌락 근처에서 같은 자세가 프레임마다
	// 다른 숫자로 표현돼 보간이 제자리를 못 찾는다
	WeaponMesh->SetRelativeRotation(FQuat::Slerp(WeaponMesh->GetRelativeRotation().Quaternion(), TargetQuat, Alpha));
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
	
	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaSeconds, AimBlendSpeed));
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