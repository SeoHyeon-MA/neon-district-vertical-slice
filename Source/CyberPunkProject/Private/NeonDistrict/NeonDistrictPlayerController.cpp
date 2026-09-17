// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictPlayerController.h"
#include "NeonDistrict/MissionObjectiveWidget.h"

void ANeonDistrictPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (!IsLocalPlayerController() || !MissionObjectiveClass) return;
	
	MissionObjective = CreateWidget<UMissionObjectiveWidget>(this, MissionObjectiveClass);
	if (MissionObjective)
	{
		MissionObjective->AddToPlayerScreen(0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[NeonDistrict] 목표 위젯 생성 실패"));
	}
}
