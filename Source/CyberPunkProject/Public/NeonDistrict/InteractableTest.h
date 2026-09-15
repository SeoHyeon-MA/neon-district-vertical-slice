// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "InteractableTest.generated.h"

class UStaticMeshComponent;

// 상호작용 배선 확인용, E를 누르면 로그를 찍고 색이 바뀐다.
UCLASS()
class CYBERPUNKPROJECT_API AInteractableTest : public AActor, public IInteractable
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
	
	int32 InteractCount = 0;
	
public:	
	// Sets default values for this actor's properties
	AInteractableTest();

	//~ IInteractable
	virtual FText GetInteractionPrompt() const override;
	virtual void Interact(APawn* InteractingPawn) override;
};
