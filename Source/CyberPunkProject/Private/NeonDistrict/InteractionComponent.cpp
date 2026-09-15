// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/InteractionComponent.h"
#include "NeonDistrict/Interactable.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateTarget();
	// ...
}

void UInteractionComponent::UpdateTarget()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->GetController())
	{
		return;
	}
	
	// 카메라(시점) 위치와 방향
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerPawn->GetController()->GetPlayerViewPoint(ViewLocation, ViewRotation);
	
	const FVector End = ViewLocation + ViewRotation.Vector() * TraceDistance;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);
	
	FHitResult Hit;
	GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Visibility, Params);
	
	// 맞은 것이 상호작용 대상인지
	AActor* NewTarget = nullptr;
	if (Hit.GetActor() && Hit.GetActor()->Implements<UInteractable>())
	{
		NewTarget = Hit.GetActor();
	}
	// 바뀌었을 떄만 방송
	if (NewTarget != CurrentTarget.Get())
	{
		CurrentTarget = NewTarget;
		OnTargetChanged.Broadcast(NewTarget);
	}
}

void UInteractionComponent::TryInteract()
{
	AActor* Target = CurrentTarget.Get();
	if (!Target)
	{
		return;
	}
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	
	if (IInteractable* Interactable = Cast<IInteractable>(Target))
	{
		if (Interactable->CanInteract(OwnerPawn))
		{
			Interactable->Interact(OwnerPawn);
		}
	}
}


