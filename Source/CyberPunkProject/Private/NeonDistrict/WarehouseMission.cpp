// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/WarehouseMission.h"

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
	
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 단계 -> %s"), *GetObjectiveText().ToString());
	
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

