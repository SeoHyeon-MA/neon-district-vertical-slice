// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NeonDistrictMainMission.h"
#include "NeonDistrict/NeonDistrictMission.h"
#include "WarehouseMission.generated.h"

class AShooterNPC;
class AWarehousePickup;
// 이 미션 안에서만 의미 있는 세부 단계
UENUM()
enum class EWarehouseStep : uint8
{
	Fighting,			// 적 처리 중				"적을 처리하세요"
	KeyDropped,			// 마지막 적이 키를 떨어트림	"창고 키를 획득하세요"
	KeyAcquired,		// 키 소지, 창고 잠김			"창고 문을 여세요"
	ItemAcquired		// 아이템 획득, 셔터 개방		"Fixer에게 돌아가세요"
};

/**
 * 적을 처리하고 창고 키를 얻어 아이템을 회수한 뒤 Fixer에게 돌아오는 미션 
 */
UCLASS()
class CYBERPUNKPROJECT_API AWarehouseMission : public ANeonDistrictMainMission
{
	GENERATED_BODY()
	
protected:
	// 마지막 적이 죽으면 그 자리에 스폰할 키
	UPROPERTY(EditAnywhere, Category="Neon District")
	TSubclassOf<AWarehousePickup> KeyClass;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Neon District")
	EWarehouseStep Step = EWarehouseStep::Fighting;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District")
	int32 AliveEnemies = 0;
	
	// 스폰한 키. 구간을 되돌릴 때 같이 치운다
	TWeakObjectPtr<AWarehousePickup> DroppedKey;
	
	virtual void SetupSegment() override;

public:
	// 세부 단계를 진행시킨다. 키 드롭, 아이템 획득 등이 부른다
	void AdvanceTo(EWarehouseStep NewStep);
	
	// 적이 생길 때 부른다. 사망을 구독하고 수를 센다
	void RegisterEnemy(AShooterNPC* Enemy);
	
	EWarehouseStep GetStep() const {return Step;}
	
	virtual FText GetObjectiveText() const override;
	
	virtual bool CanComplete() const override;
	
	// 진행 중인 창고 미션. 등록부에서 찾는다. 없으면 nullptr
	static AWarehouseMission* FindActive(const UObject* WorldContext);
	
private:
	// OnPawnDeath가 다이나믹 델리게이트라 UFUNCTION이어야 한다
	UFUNCTION()
	void HandleEnemyDied();
	
	void DropKey(const FVector& Location);
};
