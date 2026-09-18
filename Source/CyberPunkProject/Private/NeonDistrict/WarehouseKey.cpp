// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/WarehouseKey.h"
#include "NeonDistrict/MissionRegistry.h"
#include "NeonDistrict/WarehouseMission.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWarehouseKey::AWarehouseKey()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionObjectType(ECC_WorldDynamic);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 상호작용 트레이스만

}

FText AWarehouseKey::GetInteractionPrompt() const
{
	return FText::FromString(TEXT("창고 키 줍기"));
}

bool AWarehouseKey::CanInteract(APawn* InteractingPawn) const
{
	// 키가 떨어진 단계에서만. 그 전엔 바닥에 있어도 못 줍는다
	const AWarehouseMission * Mission = AWarehouseMission::FindActive(this);
	return Mission && Mission->GetStep() == EWarehouseStep::KeyDropped;
}

void AWarehouseKey::Interact(APawn* InteractingPawn)
{
	if (AWarehouseMission* Mission = AWarehouseMission::FindActive(this))
	{
		Mission->AdvanceTo(EWarehouseStep::KeyAcquired);
		Destroy();
	}
}

