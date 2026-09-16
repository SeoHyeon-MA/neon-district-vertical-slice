// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NeonDistrict/MissionTypes.h"
#include "MissionRegistry.generated.h"

class ANeonDistrictMission;

// 미션이 등록되거나(true) 빠질 때(false) 방송
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActiveMissionsChanged, ANeonDistrictMission*, bool /*bResigstered*/);

/**
 * 지금 진행 중인 미션을 찾는 통로. 상태는 들지 않는다
 */
UCLASS()
class CYBERPUNKPROJECT_API UMissionRegistry : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	FOnActiveMissionsChanged OnActiveMissionsChanged;
	
	//해당 분류에서 진행 중인 첫 미션. 메인은 하나. 없으면 nullptr
	ANeonDistrictMission* GetActiveMission(EMissionCategory Category) const;
	//해당 분류에서 진행 중인 미션 전부. 사이드 목록용
	void GetActiveMissions(EMissionCategory Category, TArray<ANeonDistrictMission*>& OutMissions) const;
	
	//미션이 시작할 때 부른다
	void Register(ANeonDistrictMission* Mission);
	//미션이 끝날 때 부른다
	void Unregister(ANeonDistrictMission* Mission);

private:
	TArray<TWeakObjectPtr<ANeonDistrictMission>> ActiveMissions;
};
