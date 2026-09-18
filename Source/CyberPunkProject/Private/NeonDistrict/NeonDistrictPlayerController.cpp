// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictPlayerController.h"
#include "NeonDistrict/MissionObjectiveWidget.h"
#include "NeonDistrict/DialogueWidget.h"

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

bool ANeonDistrictPlayerController::IsInDialogue() const
{
	return Dialogue && Dialogue->IsActive();
}

UDialogueWidget* ANeonDistrictPlayerController::StartDialogue(UDataTable* Table, FName StartRow)
{
	if (!DialogueClass || IsInDialogue()) return nullptr;
	
	// 위젯은 한 번 만들어 재사용
	if (!Dialogue)
	{
		Dialogue = CreateWidget<UDialogueWidget>(this, DialogueClass);
		if (!Dialogue) return nullptr;
		Dialogue->AddToPlayerScreen(1);
		Dialogue->OnFinished.BindUObject(this, &ANeonDistrictPlayerController::HandleDialogueFinished);
	}
	
	// 대화 중엔 서서 듣는다  --> (수정 예정) 카메라 무빙 추가 - 미션마다 카메라 위치 다르게 설정할수 있는지? 캐릭터에 붙여야하는지?
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	
	Dialogue->Start(Table, StartRow);
	return Dialogue;
}

void ANeonDistrictPlayerController::AdvanceDialogue()
{
	if (IsInDialogue())
	{
		Dialogue->Advance();
	}
}

void ANeonDistrictPlayerController::HandleDialogueFinished()
{
	SetIgnoreMoveInput(false);
	SetIgnoreLookInput(false);
}
