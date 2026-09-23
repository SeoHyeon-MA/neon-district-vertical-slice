// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrict/Interactable.h"
#include "NeonDistrict/MissionDialogueTypes.h"
#include "FixerNPC.generated.h"

class UCameraComponent;
class ANeonDistrictMission;
class UCapsuleComponent;
class UDataTable;
/*
 * 미션을 주는 NPC. E키로 말을 걸면 연결된 미션을 수락시킨다.
 */
UCLASS()
class CYBERPUNKPROJECT_API AFixerNPC : public AActor, public IInteractable
{
	GENERATED_BODY()
	
	//상호작용 트레이스가 맞는 몸통. Visibility 채널만 막는다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	UCapsuleComponent* Capsule;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* Mesh;

	// 대화 중 화면을 잡는 카메라. 뷰포트에서 각도를 맞춘다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* DialogueCamera;
	
protected:
	// 이 NPC가 주는 미션, 레벨에서 지정
	UPROPERTY(EditInstanceOnly, Category="Neon District")
	TObjectPtr<ANeonDistrictMission> Mission;
	
	// 대사 테이블. 행 구조는 FDialogueLine
	UPROPERTY(EditAnywhere, Category = "Neon District|Dialogue", meta = (RequiredAssetDataTags = "RowStructure=/Script/CyberPunkProject.DialogueLine"))
	TObjectPtr<UDataTable> DialogueTable;
	
	// 미션 상태별 시작 행
	UPROPERTY(EditAnywhere, Category = "Neon District|Dialogue")
	FName IntroRow = TEXT("Intro_1");
	
	UPROPERTY(EditAnywhere, Category = "Neon District|Dialogue")
	FName InProgressRow = TEXT("InProgress_1");
	
	UPROPERTY(EditAnywhere, Category = "Neon District|Dialogue")
	FName CompletedRow = TEXT("Completed_1");
	
	UPROPERTY(EditAnywhere, Category = "Neon District|Dialogue")
	FName ReturnRow = TEXT("Return_1");
	
public:	
	// Sets default values for this actor's properties
	AFixerNPC();

	// 대화 중 쓸 카메라. 없으면 화면을 바꾸지 않는다
	UCameraComponent* GetDialogueCamera() const { return DialogueCamera; }
	
	//~ Begin IInteractable Interface
	virtual FText GetInteractionPrompt() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IInteractable Interface

private:
	// 미션 상태로 시작 행을 고른다
	FName PickStartRow() const;
	
	// 줄의 Effect를 미션에 적용
	void ApplyEffect(EDialogueEffect Effect);
};
