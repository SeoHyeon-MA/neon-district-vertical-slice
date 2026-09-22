// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/MissionEnemyComponent.h"
#include "NeonDistrict/WarehouseMission.h"
#include "Variant_Shooter/AI/ShooterNPC.h"

// Called when the game starts
void UMissionEnemyComponent::BeginPlay()
{
	Super::BeginPlay();

	AShooterNPC* Enemy = Cast<AShooterNPC>(GetOwner());
	AWarehouseMission* Mission = AWarehouseMission::FindActive(this);
	
	if (Enemy && Mission)
	{
		Mission->RegisterEnemy(Enemy);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Enemy] 등록 실패 - 주인이 ShoterNPC가 아니거나 진행 중인 창고 미션이 없음"));
	}
	
}
