// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictPlayerController.h"
#include "NeonDistrict/MissionObjectiveWidget.h"
#include "NeonDistrict/DialogueWidget.h"
#include "NeonDistrict/InteractionPromptWidget.h"
#include "NeonDistrict/InteractionComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealthBarWidget.h"
#include "InputMappingContext.h"
#include "Variant_Shooter/ShooterCharacter.h"

void ANeonDistrictPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// HUD는 로컬 플레이어에게만
	if (!IsLocalPlayerController() || !MissionObjectiveClass) return;
	
	// 목표 문구
	MissionObjective = CreateWidget<UMissionObjectiveWidget>(this, MissionObjectiveClass);
	if (MissionObjective)
	{
		MissionObjective->AddToPlayerScreen(0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[NeonDistrict] 목표 위젯 생성 실패"));
	}
	
	// 체력 바 위젯
	HealthBar = CreateWidget<UHealthBarWidget>(this, HealthBarWidgetClass);
	if (HealthBar)
	{
		HealthBar->AddToPlayerScreen(0);
		// 첫 스폰에서는 OnPossess가 BeginPlay보다 먼저 올 수 있다. 이미 폰이 있으면 지금 묶는다
		HealthBar->BindToCharacter(GetPawn<AShooterCharacter>());
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[NeonDistrict] 체력 바 위젯 생성 실패"));
	}
	
	// 상호작용 프롬포트
	if (InteractionPromptClass)
	{
		InteractionPrompt = CreateWidget<UInteractionPromptWidget>(this, InteractionPromptClass);
		if (InteractionPrompt)
		{
			InteractionPrompt->AddToPlayerScreen(0);
			
			// 첫 스폰에서는 OnPossess가 BeginPlay보다 먼저 올 수 있다. 이미 폰이 있으면 지금 묶는다
			InteractionPrompt->BindToComponent(GetPawn() ? GetPawn()->FindComponentByClass<UInteractionComponent>() : nullptr);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[NeonDistrict] 프롬포트 위젯 생성실패"));
		}
	}
}

void ANeonDistrictPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	// 폰이 바뀔 때마다 그 폰의 상호작용 컴포넌트에 다시 묶는다
	if (InteractionPrompt)
	{
		InteractionPrompt->BindToComponent(InPawn ? InPawn->FindComponentByClass<UInteractionComponent>() : nullptr);
	}
	if (HealthBar)
	{
		HealthBar->BindToCharacter(Cast<AShooterCharacter>(InPawn));
	}
}

bool ANeonDistrictPlayerController::IsInDialogue() const
{
	return Dialogue && Dialogue->IsActive();
}

UDialogueWidget* ANeonDistrictPlayerController::StartDialogue(UDataTable* Table, FName StartRow, AActor* ViewTarget)
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
	
	// 대화 중엔 무기 입력을 끊는다. 누르고 있던 사격도 멈춘다
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Ctx : DialogueBlockedContexts)
		{
			Subsystem->RemoveMappingContext(Ctx);
		}
	}
	if (AShooterCharacter* ShooterPawn = GetPawn<AShooterCharacter>())
	{
		ShooterPawn->DoStopFiring();
	}
	
	// 대화 상대가 카메라를 주면 그쪽으로. 없으면 화면은 그대로
	if (ViewTarget)
	{
		SetViewTargetWithBlend(ViewTarget, DialogueCameraBlendTime, VTBlend_EaseInOut, 2.f);
	}
	
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
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* Ctx : DialogueBlockedContexts)
		{
			Subsystem->AddMappingContext(Ctx, 0);
		}
	}
	
	// 폰으로 복귀(카메라)
	if (APawn* MyPawn = GetPawn())
	{
		SetViewTargetWithBlend(MyPawn, DialogueCameraBlendTime, VTBlend_EaseInOut, 2.f);
	}
}
