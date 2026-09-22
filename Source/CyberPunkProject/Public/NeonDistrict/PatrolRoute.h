// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

/*
 * 순찰 경로. 점들을 액터 기준 상대 좌표로 들고, 적이 순서대로 돈다. DL_Mission 레이어에 둔다
 */
UCLASS()
class CYBERPUNKPROJECT_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
protected:
	// 순찰 지점. 액터 기준 상대 좌표. 뷰포트에서 위젯으로 끌어 놓는다
	UPROPERTY(EditAnywhere, Category="Neon District", meta=(MakeEditWidget))
	TArray<FVector> Points;
	
	// 끝에 닿으면 처음으로 (true) / 되돌아 온다 (false)
	UPROPERTY(EditAnywhere, Category="Neon District")
	bool bLoop = true;
	
public:
	APatrolRoute();
	
	int32 NumPoints() const {return Points.Num();}
	
	// 월드 좌표. 인덱스는 범위 안이어야 한다
	FVector GetPointWorld(int32 Index) const;
	// 다음 인덱스. 루프 여부와 진행 방향을 고려한다. Direction은 ±1이고 되돌아올때 뒤집힌다.
	int32 NextIndex(int32 Current, int32& Direction) const;
	//월드 위치에서 가장 가까운 점의 인덱스
	int32 NearestIndex(const FVector& WorldLocation) const;

};
