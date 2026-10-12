// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrict/WarehouseMission.h"
#include "WarehouseShutter.generated.h"

class UStaticMeshComponent;
class ANeonDistrictMission;
class AWarehouseMission;

/*
 * 복귀 지름길 셔터. 아이템을 얻으면 위로 열린다. 미션을 구독할 뿐 플레이어와 직접 상호작용하지 않는다. DL_Mission 레이어에 둔다.
 */
UCLASS()
class CYBERPUNKPROJECT_API AWarehouseShutter : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;
	
protected:
	// 이 단계 이상이 되면 열린다. 진입 셔터는 Fighting, 복귀 셔터는 ItemAcquired
	UPROPERTY(EditAnywhere, Category="Neon District")
	EWarehouseStep OpenAtStep = EWarehouseStep::ItemAcquired;
	
	// 열렸을 때 올라가는 높이 (cm)
	UPROPERTY(EditAnywhere, Category="Neon District")
	float OpenHeight = 600.f;
	
	// cm/초
	UPROPERTY(EditAnywhere, Category="Neon District")
	float OpenSpeed = 300.f;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District")
	bool bOpen = false;
	
public:	
	// Sets default values for this actor's properties
	AWarehouseShutter();

	//~ Begin AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
	
private:
	// 미션 상태가 바뀔 때. 아이템 단계 이후면 연다
	void HandleMissionStateChanged(ANeonDistrictMission* Mission);
	
	void Open();
	
	TWeakObjectPtr<AWarehouseMission> BoundMission;
};
