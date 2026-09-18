// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/ShooterPlayerController.h"
#include "NeonDistrictPlayerController.generated.h"

class UInteractionPromptWidget;
class UMissionObjectiveWidget;
class UDialogueWidget;
class UDataTable;

/**
 * Neon District 플레이어 컨트롤러. 폰이 바뀌어도 살아남는 HUD를 소유한다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API ANeonDistrictPlayerController : public AShooterPlayerController
{
	GENERATED_BODY()
	
protected:
	// 목표 문구 위젯 클래스. 에디터에서 WBP_MissionObjective 지정
	UPROPERTY(EditDefaultsOnly, Category="Neon District|UI")
	TSubclassOf<UMissionObjectiveWidget> MissionObjectiveClass;
	
	UPROPERTY()
	TObjectPtr<UMissionObjectiveWidget> MissionObjective;
	
	UPROPERTY(EditDefaultsOnly, Category="Neon District|UI")
	TSubclassOf<UInteractionPromptWidget> InteractionPromptClass;
	
	UPROPERTY()
	TObjectPtr<UInteractionPromptWidget> InteractionPrompt;
	
	UPROPERTY(EditDefaultsOnly, Category="Neon District|UI")
	TSubclassOf<UDialogueWidget> DialogueClass;
	
	UPROPERTY()
	TObjectPtr<UDialogueWidget> Dialogue;
	
public:
	bool IsInDialogue() const;
	
	//대화를 열고 위젯을 돌려준다. 부른 쪽이 OnEffect를 묶는다
	UDialogueWidget* StartDialogue(UDataTable* Table, FName StartRow);
	
	void AdvanceDialogue();
	
protected:
	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	//~ End AActor Interface
	
	//~ Begin AController Interface
	virtual void OnPossess(APawn* InPawn) override;
	//~ End AController Interface
	
private:
	void HandleDialogueFinished();
}; 
