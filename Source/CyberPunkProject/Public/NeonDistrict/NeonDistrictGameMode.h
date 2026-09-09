#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "NeonDistrictGameMode.generated.h"

class AShooterWeapon;

UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictGameMode : public AShooterGameMode
{
	GENERATED_BODY()
	
protected:
	
	//플레이어가 게임 시작 시 지급받는 무기
	UPROPERTY(EditDefaultsOnly, Category="Neon District")
	TSubclassOf<AShooterWeapon> StartingWeaponClass;
	
public:
	
	ANeonDistrictGameMode();
	
protected:
	
	//플레이어가 스폰되어 폰에 빙의한 직후 호출된다
	/** 폰·컨트롤러·UI 클래스는 생성자가 아니라 여기서 로드한다 */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** 플레이어가 스폰되어 폰에 빙의한 직후 호출된다 */
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
};