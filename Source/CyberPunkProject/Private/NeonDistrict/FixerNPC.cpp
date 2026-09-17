// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/FixerNPC.h"
#include "NeonDistrict/NeonDistrictMission.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AFixerNPC::AFixerNPC()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(Capsule);
	Capsule->InitCapsuleSize(34.f, 88.f);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	Capsule->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); //상호작용 트레이스
	Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); //플레이어가 뚫고 지나가지 않게

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Capsule);
	Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	Mesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

FText AFixerNPC::GetInteractionPrompt() const
{
	return FText::FromString(TEXT("Fixer와 대화"));
}

bool AFixerNPC::CanInteract(APawn* InteractingPawn) const
{
	return Mission != nullptr;	
}

void AFixerNPC::Interact(APawn* InteractingPawn)
{
	//지금은 바로 수락. 대화 UI가 들어오면 대화가 끝난 뒤로 옮긴다
	Mission->AcceptMission();
}
