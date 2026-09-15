// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrictMission.generated.h"

class ANeonDistrictMission;
class UBoxComponent;
class UArrowComponent;
class ANeonDistrictCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMissionStateChanged, ANeonDistrictMission*);

UENUM()
enum class EMissionState : uint8
{
	NotAccepted,
	Accepted,
	InProgress,
	Completed
};

UCLASS(Abstract)
class CYBERPUNKPROJECT_API ANeonDistrictMission : public AActor
{
	GENERATED_BODY()
	
	/* 미션이 시작되는 구역 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* EntryTrigger;
	
	/* 리스폰 위치. (트리거 바깥) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UArrowComponent* RestartPoint;
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Neon District")
	EMissionState State = EMissionState::NotAccepted;
	
	//진행 중 죽은 횟수, 랭크 계산에 사용
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Neon District")
	int32 DeathCount = 0;
	
	
public:	
	// Sets default values for this actor's properties
	ANeonDistrictMission();

	// NPC 대화가 끝나면 호출
	void AcceptMission();
	
	// 상태나 세부 단계가 바뀔때 방송 - HUD,창고 문, NPC가 구독
	FOnMissionStateChanged OnStateChanged;
	
	// 지금 플레이어가 해야 할 일. 자식이 세부 단계에 따라 답한다.
	virtual FText GetObjectiveText() const;
	
	// 미션 진행 중인가
	bool IsInProgress() const { return State == EMissionState::InProgress; }
	
	// 사망 횟수로 계산한 랭크 - S:0 / A:1 / B:2 / C:3이상
	FText GetRank() const;
	
	// 미션 완료 - 자식이 조건을 판단해 부른다
	void CompleteMission();
	
protected:
	/* 구역 진입 감지 */
	UFUNCTION()
	void OnEntryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/* 미션 구간 시작 */
	void StartMission(APawn* Player);
	
	/** 새 폰이 생길 때마다 죽음 이벤트를 구독한다 */
	void BindToPlayer(APawn* Player);

	/** 플레이어 죽음 처리 */
	void HandlePlayerDied(ANeonDistrictCharacter* Character);

	/** 구간을 처음 상태로 세팅한다. 시작할 때와 죽은 뒤에 같이 쓴다 */
	virtual void SetupSegment();
	
	// 상태가 바뀌었음을 알린다. 자식이 세부 단계를 바꾼 뒤 부름
	void NotifyStateChanged();

};
