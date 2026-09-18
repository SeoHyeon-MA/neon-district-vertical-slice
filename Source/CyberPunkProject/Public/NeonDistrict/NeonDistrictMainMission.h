// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NeonDistrict/NeonDistrictMission.h"
#include "NeonDistrictMainMission.generated.h"

class UArrowComponent;
class ANeonDistrictCharacter;
class UDataLayerAsset;

/**
 * 메인 미션. 체크포인트를 소유하고, 사망을 세어 구간을 되돌리고, 랭크를 매긴다.
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API ANeonDistrictMainMission : public ANeonDistrictMission
{
	GENERATED_BODY()
	
	//리스폰 위치 (트리거 바깥)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* RestartPoint;
	
protected:
	//구간에 속한 액터(적.키.문)가 들어 있는 런타임 데이터 레이어. 재활성화로 초기화한다
	UPROPERTY(EditAnywhere, Category="Neon District")
	TObjectPtr<const UDataLayerAsset> SegmentLayer;
	
	//진행 중 죽은 횟수. 랭크 계산에 사용
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category= "Neon District")
	int32 DeathCount = 0;
	
public:
	ANeonDistrictMainMission();
	
	int32 GetDeathCount() const { return DeathCount; }
	
	// 랭크 (사망 횟수로 계산 후 text 반환) - S:0 / A:1 / B:2 / C:3이상
	FText GetRank() const;
	
protected:
	//~ Begin ANeonDistrictMission Interface
	virtual void StartMission(APawn* Player) override;
	virtual FText GetObjectiveText() const override;
	//~ End ANeonDistrictMission Interface
	
	// 구간 세팅. 시작할 때와 죽을 때마다. 자식이 적 배치 등을 덮어쓴다
	virtual void SetupSegment();
	
private:
	//새 Pawn마다 사망 방송을 구독한다
	void BindToPlayer(APawn* Player);
	
	void HandlePlayerDied(ANeonDistrictCharacter* Character);
	
	// 레이어가 내려간 걸 확인한 뒤 다시 올린다. SetupSegment가 타이머로 반복 호출
	void ActiveSegmentWhenUnloaded();
	
	FTimerHandle SegmentReloadTimer;

	// 내려가는 중인 구간 셀의 레벨 패키지 이름. GC로 사라졌는지 확인하는 데 쓴다
	TArray<FName> SegmentCellPackages;
};
