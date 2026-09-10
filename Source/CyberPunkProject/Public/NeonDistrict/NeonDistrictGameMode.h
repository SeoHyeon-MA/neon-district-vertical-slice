#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterGameMode.h"
#include "NeonDistrictGameMode.generated.h"

class AShooterWeapon;
class ANeonDistrictMission;

UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictGameMode : public AShooterGameMode
{
	GENERATED_BODY()
	
protected:
	
	//플레이어가 게임 시작 시 지급받는 무기
	UPROPERTY(EditDefaultsOnly, Category="Neon District")
	TArray<TSubclassOf<AShooterWeapon>> StartingWeapons;

	// 죽으면 여기서 다시 시작 - 설정 전에는 PlayerStart를 사용
	FTransform Checkpoint;
	bool bHasCheckpoint = false;
	
	//지금 진행 중인 미션
	TWeakObjectPtr<ANeonDistrictMission> ActiveMission;
	
public:
	
	ANeonDistrictGameMode();

	// 미션 진행에 따라 재시작 지점을 옮긴다.
	void SetCheckpoint(const FTransform& NewCheckpoint);
	// 미션이 시작될 때 자기를 등록한다
	void SetActiveMission(ANeonDistrictMission* Mission);

	
protected:
	
	//플레이어가 스폰되어 폰에 빙의한 직후 호출된다
	/** 폰·컨트롤러·UI 클래스는 생성자가 아니라 여기서 로드한다 */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** 폰이 스폰될 때마다 호출된다 (최초 스폰과 부활 모두) */
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	/* 폰이 필요할 때마다 호출. 체크포인트가 있으면 그쪽으로 보낸다 */
	virtual void RestartPlayer(AController* NewPlayer) override;
	
};