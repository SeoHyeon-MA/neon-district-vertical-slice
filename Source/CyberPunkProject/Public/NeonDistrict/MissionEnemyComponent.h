// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MissionEnemyComponent.generated.h"

/*
 * 창고 구간의 적에 붙인다. 생기면 진행 중인 창고 미션에 주인(AShooterNPC)을 등록한다
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERPUNKPROJECT_API UMissionEnemyComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface
		
};
