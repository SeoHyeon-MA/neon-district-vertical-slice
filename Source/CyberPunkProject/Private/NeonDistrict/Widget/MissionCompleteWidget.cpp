// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/MissionCompleteWidget.h"

#include "MissionRegistry.h"
#include "NeonDistrictMainMission.h"
#include "NeonDistrictMission.h"

void UMissionCompleteWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetVisibility(ESlateVisibility::Collapsed);
	
	UMissionRegistry* Registry = GetWorld()->GetSubsystem<UMissionRegistry>();
	if (!Registry) return;
	
	BoundRegistry = Registry;
	Registry->OnActiveMissionsChanged.AddUObject(this, &UMissionCompleteWidget::HandleActiveMissionChanged);
}

void UMissionCompleteWidget::NativeDestruct()
{
	if (BoundRegistry.IsValid())
	{
		BoundRegistry->OnActiveMissionsChanged.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UMissionCompleteWidget::HandleActiveMissionChanged(ANeonDistrictMission* Mission, bool bRegistered)
{
	// 등록이 아니라 해제일 때만. 중도 포기로 빠지는 경우를 대비해 완료 상태도 확인한다
	if (bRegistered || !Mission || Mission->GetState() != EMissionState::Completed)  return;
	
	// 랭크는 메인 미션만 매긴다. 사이드는 창을 띄우지 않는다
	const ANeonDistrictMainMission* MainMission = Cast<ANeonDistrictMainMission>(Mission);
	if (!MainMission) return;
	
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	BP_ShowResult(MainMission->GetRank(), MainMission->GetDeathCount());
}
