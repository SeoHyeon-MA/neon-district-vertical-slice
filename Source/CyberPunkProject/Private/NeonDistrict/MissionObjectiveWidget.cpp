// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/MissionObjectiveWidget.h"

#include "NeonDistrict/MissionRegistry.h"
#include "NeonDistrict/NeonDistrictMission.h"

void UMissionObjectiveWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	//아직 미션이 없으면 보이지 않는다
	SetVisibility(ESlateVisibility::Collapsed);
	
	UMissionRegistry* Registry = GetWorld()->GetSubsystem<UMissionRegistry>();
	if (!Registry) return;
	
	BoundRegistry = Registry;
	Registry->OnActiveMissionsChanged.AddUObject(this, &UMissionObjectiveWidget::HandleActiveMissionsChanged);
	
	// 위젯보다 먼저 시작한 미션이 있을 수 있다.
	TrackMission(Registry->GetActiveMission(TrackedCategory));
}

void UMissionObjectiveWidget::NativeDestruct()
{
	if (BoundRegistry.IsValid())
	{
		BoundRegistry->OnActiveMissionsChanged.RemoveAll(this);
	}
	TrackMission(nullptr);
	
	Super::NativeDestruct();
}

void UMissionObjectiveWidget::HandleActiveMissionsChanged(ANeonDistrictMission* Mission, bool bRegistered)
{
	if (!Mission || Mission->GetCategory() != TrackedCategory) return;
	
	if (bRegistered)
	{
		TrackMission(Mission);
	}
	else if (Mission == BoundMission.Get())
	{
		// 보고 있던 미션이 끝났다. 구독만 끊고 마지막 문구("미션 완료")는 남긴다
		TrackMission(nullptr);
	}
}

void UMissionObjectiveWidget::HandleMissionStateChanged(ANeonDistrictMission* Mission)
{
	Refresh();
}

void UMissionObjectiveWidget::TrackMission(ANeonDistrictMission* Mission)
{
	if (BoundMission.IsValid())
	{
		BoundMission->OnStateChanged.RemoveAll(this);
	}
	BoundMission = Mission;
	if (!Mission) return;
	
	Mission->OnStateChanged.AddUObject(this, &UMissionObjectiveWidget::HandleMissionStateChanged);
	Refresh();
}

void UMissionObjectiveWidget::Refresh()
{
	if (!BoundMission.IsValid()) return;
	
	BP_UpdateObjective(BoundMission->GetObjectiveText());
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
