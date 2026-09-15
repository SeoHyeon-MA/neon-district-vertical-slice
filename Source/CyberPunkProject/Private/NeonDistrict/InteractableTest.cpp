// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/InteractableTest.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

// Sets default values
AInteractableTest::AInteractableTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

FText AInteractableTest::GetInteractionPrompt() const
{
	return FText::FromString(TEXT("테스트 상자 누르기"));
}

void AInteractableTest::Interact(APawn* InteractingPawn)
{
	++InteractCount;
	UE_LOG(LogTemp, Warning, TEXT("[Interact] %s가 %d번째 상호작용"), *GetNameSafe(InteractingPawn), InteractCount);
}

