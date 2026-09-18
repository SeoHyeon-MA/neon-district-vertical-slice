// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NeonDistrictMainMission.h"
#include "Materials/MaterialExpressionOperator.h"
#include "NeonDistrict/NeonDistrictMission.h"
#include "WarehouseMission.generated.h"

// 이 미션 안에서만 의미 있는 세부 단계
UENUM()
enum class EWarehouseStep : uint8
{
	Fighting, // 적 처리 중
	KeyDropped, // 마지막 적이 키를 떨어트림
	ItemAcquired, // 창고에서 아이템 획득
	Returning // Fixer에게 복귀 중
};

/**
 * 적을 처리하고 창고 키를 얻어 아이템을 회수한 뒤 Fixer에게 돌아오는 미션 
 */
UCLASS()
class CYBERPUNKPROJECT_API AWarehouseMission : public ANeonDistrictMainMission
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Neon District")
	EWarehouseStep Step = EWarehouseStep::Fighting;
	
	virtual void SetupSegment() override;

public:
	// 세부 단계를 진행시킨다. 키 드롭, 아이템 획득 등이 부른다
	void AdvanceTo(EWarehouseStep NewStep);
	
	EWarehouseStep GetStep() const {return Step;}
	
	virtual FText GetObjectiveText() const override;
	
	virtual bool CanComplete() const override;
	
};
