// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrictMission.generated.h"

class UBoxComponent;

UCLASS()
class CYBERPUNKPROJECT_API ANeonDistrictMission : public AActor
{
	GENERATED_BODY()
	
	/* 미션이 시작되는 구역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* EntryTrigger;
	
protected:
	/* NPC에게 미션을 받았는지 (false일 경우 구역에 들어와도 아무일도 일어나지 않음) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Neon District")
	bool bAccepted = false;
	
	/* 미션 구간이 진행 중인가 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Neon District")
	bool bRunning = false;
	
	/* 몇 번째 시도인지 - 랭크 계산에 사용 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Neon District")
	int32 AttemptCount = 0;
	
public:	
	// Sets default values for this actor's properties
	ANeonDistrictMission();

	// NPC 대화가 끝나면 호출
	void AcceptMission();


protected:
	/* 구역 진입 감지 */
	UFUNCTION()
	void OnEntryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/* 미션 구간 시작 */
	void StartMission(APawn* Player);

};
