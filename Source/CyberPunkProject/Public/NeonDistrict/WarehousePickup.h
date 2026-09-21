// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WarehouseMission.h"
#include "GameFramework/Actor.h"
#include "NeonDistrict/Interactable.h"
#include "WarehousePickup.generated.h"

class UStaticMeshComponent;

/*
 * 창고 키. 주우면 미션이 KeyAcquired로 넘어가고 사라진다. DL_Mission 레이어에 둔다
 */
UCLASS()
class CYBERPUNKPROJECT_API AWarehousePickup : public AActor, public IInteractable
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;
	
protected:
	UPROPERTY(EditAnywhere, Category="Neon District")
	FText Prompt;
	
	// 이 단계일 때만 주울 수 있다
	UPROPERTY(EditAnywhere, Category="Neon District")
	EWarehouseStep RequiredStep = EWarehouseStep::KeyDropped;
	
	// 주우면 이 단계로
	UPROPERTY(EditAnywhere, Category="Neon District")
	EWarehouseStep NextStep = EWarehouseStep::KeyAcquired;
	
public:	
	// Sets default values for this actor's properties
	AWarehousePickup();
	
	//~ Begin IInteractable Interface
	virtual FText GetInteractionPrompt() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IInteractable Interface
};
