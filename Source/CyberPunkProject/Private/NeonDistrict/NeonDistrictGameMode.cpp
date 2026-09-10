#include "NeonDistrict/NeonDistrictGameMode.h"

#include "NeonDistrictPistol.h"
#include "NeonDistrictRifle.h"
#include "NeonDistrict/NeonDistrictWeapon.h"
#include "UObject/ConstructorHelpers.h"                    // ← ConstructorHelpers::FClassFinder
#include "Variant_Shooter/Weapons/ShooterWeaponHolder.h"   // ← IShooterWeaponHolder
#include "GameFramework/PlayerController.h"                // ← WeakPC->GetPawn()
#include "GameFramework/Pawn.h"
#include "Variant_Shooter/UI/ShooterUI.h"                            // ← FClassFinder<APawn>
#include "TimerManager.h"
#include "Engine/World.h"

ANeonDistrictGameMode::ANeonDistrictGameMode()
{
	//시작 무기를 만든 총으로
	StartingWeapons.Add(ANeonDistrictPistol::StaticClass());
	StartingWeapons.Add(ANeonDistrictRifle::StaticClass());
}

void ANeonDistrictGameMode::SetCheckpoint(const FTransform& NewCheckpoint)
{
	Checkpoint = NewCheckpoint;
	bHasCheckpoint = true;
}

void ANeonDistrictGameMode::RestartPlayer(AController* NewPlayer)
{
	if (bHasCheckpoint && NewPlayer)
	{
		RestartPlayerAtTransform(NewPlayer, Checkpoint);
		return;
	}
	
	// 체크포인트가 없으면 원래대로 PlayerStart 사용
	Super::RestartPlayer(NewPlayer);
}

void ANeonDistrictGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// 템플릿 셔터 캐릭터 (메시와 애니메이션이 들어 있다)
	if (UClass* PawnClass = LoadClass<APawn>(nullptr,
		TEXT("/Game/NeonDistrict/Blueprints/BP_NeonDistrictCharacter.BP_NeonDistrictCharacter_C")))
	{
		DefaultPawnClass = PawnClass;
	}

	// 입력 매핑과 카메라 설정이 들어 있는 컨트롤러
	if (UClass* PCClass = LoadClass<APlayerController>(nullptr,
		TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController.BP_ShooterPlayerController_C")))
	{
		PlayerControllerClass = PCClass;
	}

	// 탄약 카운터 UI
	if (UClass* UIClass = LoadClass<UShooterUI>(nullptr,
		TEXT("/Game/Variant_Shooter/UI/UI_Shooter.UI_Shooter_C")))
	{
		ShooterUIClass = UIClass;
	}
}

void ANeonDistrictGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
	
	if (StartingWeapons.IsEmpty())
	{
		return;
	}
	
	// 폰과 애니메이션 초기화가 끝난 다음 프레임에 지급한다.
	TWeakObjectPtr<APawn> WeakPawn(PlayerPawn);
	
	GetWorldTimerManager().SetTimerForNextTick([this,WeakPawn]()
	{
		if (!WeakPawn.IsValid()) return;
		
		if (IShooterWeaponHolder* WeaponHolder = Cast<IShooterWeaponHolder>(WeakPawn.Get()))
		{
			for (const TSubclassOf<AShooterWeapon>& WeaponClass : StartingWeapons)
			{
				if (WeaponClass)
				{
					WeaponHolder->AddWeaponClass(WeaponClass);
				}
			}
		}
	});
}


