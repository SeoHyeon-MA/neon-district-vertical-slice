// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrict/MissionTypes.h"
#include "NeonDistrictMission.generated.h"

class ANeonDistrictMission;
class UBoxComponent;

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
	
	
protected:
	// 메인/사이드. 클래스에서 지정
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Neon District")
	EMissionCategory Category = EMissionCategory::Main;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Neon District")
	EMissionState State = EMissionState::NotAccepted;
	
	
public:	
	// Sets default values for this actor's properties
	ANeonDistrictMission();
	
	// 공개 API
	EMissionCategory GetCategory() const { return Category; }
	EMissionState GetState() const { return State; }
	
	// NPC 대화가 끝나면 호출
	void AcceptMission();
	
	// 상태나 세부 단계가 바뀔때 방송 - HUD,창고 문, NPC가 구독
	FOnMissionStateChanged OnStateChanged;
	
	// 지금 플레이어가 해야 할 일. 자식이 세부 단계에 따라 답한다.
	virtual FText GetObjectiveText() const;
	
	// 미션 진행 중인가
	bool IsInProgress() const { return State == EMissionState::InProgress; }
	
	// 완료 조건을 채웠는가. 자식이 세부 단계로 답한다
	virtual bool CanComplete() const { return false; }
	
	// 미션 완료 - 자식이 조건을 판단해 부른다
	void CompleteMission();
	
protected:
	/* 구역 진입 감지 */
	UFUNCTION()
	void OnEntryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	/* 미션 구간 시작 */
	virtual void StartMission(APawn* Player);
	
	// 상태가 바뀌었음을 알린다. 자식이 세부 단계를 바꾼 뒤 부름
	void NotifyStateChanged();

};
