// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/WarehouseMission.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"

void AWarehouseMission::SetupSegment()
{
	Super::SetupSegment();
	
	//구간을 처음 상태로 되돌린다
	Step = EWarehouseStep::Fighting;
	
	//다음 단계에서 여기에 적 정리/생성, 키/창고 원상복구가 들어간다
	UE_LOG(LogTemp, Warning, TEXT("[Warehouse] 구간 세팅 - 적 배치 예정"));
}

void AWarehouseMission::AdvanceTo(EWarehouseStep NewStep)
{
	// 진행 중이 아니면 무시한다
	if (!IsInProgress() || Step == NewStep) return;
	Step = NewStep;
	
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 단계 -> %d = %s / %s"), 
		static_cast<int32>(Step),
		*UEnum::GetValueAsString(Step),
		*GetObjectiveText().ToString());
	
	NotifyStateChanged();
}

FText AWarehouseMission::GetObjectiveText() const
{
	// 진행 중이 아닐 때는 부모의 공통 문구를 쓴다
	if (!IsInProgress())
	{
		return Super::GetObjectiveText();
	}
	
	//ItemAcquired와 Returning의 문구가 같습니다. 지금은 그렇고, 나중에 "셔터가 열렸다" 같은 구분이 필요해지면 나눕니다
	switch (Step)
	{
	case EWarehouseStep::Fighting: return FText::FromString(TEXT("적을 처리하세요"));
	case EWarehouseStep::KeyDropped: return FText::FromString(TEXT("창고 키를 획득하세요"));
	case EWarehouseStep::ItemAcquired: return FText::FromString(TEXT("Fixer에게 돌아가세요"));
	case EWarehouseStep::Returning: return FText::FromString(TEXT("Fixer에게 돌아가세요"));
	default: return FText::GetEmpty();
	}
}

//####################################디버그 설정################################
#if !UE_BUILD_SHIPPING

//배치된 창고 미션의 세부 단계를 강제로 바꾼다
static void NDSetMissionStep(const TArray<FString>& Args, UWorld* World)
{
	if (!World || Args.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("사용법: ND.SetMissionStep <0=Fighting 1=KeyDropped 2=ItemAcquired 3=Returning"));
		return;
	}
	
	const int32 Value = FCString::Atoi(*Args[0]);
	
	if (Value < 0 || Value > 3)
	{
		UE_LOG(LogTemp, Warning, TEXT("0~3 사이 값을 넣으세요"));
		return;
	}
	
	for (TActorIterator<AWarehouseMission> It(World); It; ++It)
	{
		It->AdvanceTo(static_cast<EWarehouseStep>(Value));
	}
}

static void NDMissionStatus(UWorld* World)
{
	if (!World)
	{
		return;
	}
	
	for (TActorIterator<AWarehouseMission> It(World); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] %s | State = %s | Step = %d %s | Deaths = %d | 목표: %s"),
			*It->GetName(),
			*UEnum::GetValueAsString(It->GetState()),
			static_cast<int32>(It->GetStep()),
			*UEnum::GetValueAsString(It->GetState()),
			It->GetDeathCount(),
			*It->GetObjectiveText().ToString()
			);
	}
}

static FAutoConsoleCommandWithWorldAndArgs GNDSetMissionStepCmd(
TEXT("ND.SetMissionStep"),
TEXT("창고 미션의 세부 단계를 강제로 바꾼다 (디버그용). 0=Fighting 1=KeyDropped 2=ItemAcquired 3=Returning"),
FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&NDSetMissionStep));

static FAutoConsoleCommandWithWorld GNDMissionStatusCmd(
	TEXT("ND.MissionStatus"),
	TEXT("창고 미션의 현재 상태·단계·사망 횟수를 찍는다 (디버그용)"),
	FConsoleCommandWithWorldDelegate::CreateStatic(&NDMissionStatus));

#endif