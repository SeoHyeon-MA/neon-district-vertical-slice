// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/PatrolRoute.h"
#include "Components/BillboardComponent.h"

APatrolRoute::APatrolRoute()
{
	PrimaryActorTick.bCanEverTick = false;

	// 에디터에서 보이게. 게임에서는 안 보임
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
#if WITH_EDITORONLY_DATA
	UBillboardComponent* Icon = CreateDefaultSubobject<UBillboardComponent>(TEXT("Icon"));
	Icon->SetupAttachment(Root);
#endif
}

FVector APatrolRoute::GetPointWorld(int32 Index) const
{
	return
	GetActorTransform().TransformPosition(Points[Index]);
}

int32 APatrolRoute::NextIndex(int32 Current, int32& Direction) const
{
	const int32 Num = Points.Num();
	if (Num <= 1) return 0;
	
	int32 Next = Current + Direction;
	
	if (bLoop)
	{
		return (Next + Num) % Num;
	}
	
	// 왕복 : 끝에서 방향 반대로
	if (Next < 0 || Next >= Num)
	{
		Direction = -Direction;
		Next = Current + Direction;
	}
	return Next;
}

int32 APatrolRoute::NearestIndex(const FVector& WorldLocation) const
{
	int32 Best = 0;
	double BestDistSq = TNumericLimits<double>::Max();
	for (int32 i = 0; i < Points.Num(); ++i)
	{
		const double DistSq = FVector::DistSquared(GetPointWorld(i), WorldLocation);
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = i;
		}
	}
	return Best;
}

