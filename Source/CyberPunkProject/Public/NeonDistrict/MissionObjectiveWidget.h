// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NeonDistrict/MissionTypes.h"
#include "MissionObjectiveWidget.generated.h"

class ANeonDistrictMission;
class UMissionRegistry;

/**
 * 한 분류의 진행 중인 미션 목표 문구를 보여준다. 등록부에서 미션을 찾고, 미션의 상태 변화를 구독한다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UMissionObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	//이 위젯이 보여줄 분류. WBP 자식이 정한다
	UPROPERTY(EditDefaultsOnly, Category="Neon District")
	EMissionCategory TrackedCategory = EMissionCategory::Main;
	
	// 블루프린트가 텍스트 블록을 갱신
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "UpdateObjective"))
	void BP_UpdateObjective(const FText& Objective);
	
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// 등록부에 미션이 들어오거나 빠질 때, 자기 분류만 본다
	void HandleActiveMissionsChanged(ANeonDistrictMission* Mission, bool bRegistered);
	
	// 보고 있는 미션의 상태가 바뀔 때
	void HandleMissionStateChanged(ANeonDistrictMission* Mission);
	
	// 볼 미션을 바꾼다. nullptr이면 구독만 끊는다
	void TrackMission(ANeonDistrictMission* Mission);
	
	// 보고 있는 미션에게 문구를 물어 화면에 올린다.
	void Refresh();
	
	TWeakObjectPtr<UMissionRegistry> BoundRegistry;
	TWeakObjectPtr<ANeonDistrictMission> BoundMission;
};
