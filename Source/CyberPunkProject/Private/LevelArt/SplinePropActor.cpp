// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelArt/SplinePropActor.h"

#include "LevelArt/PropMeshBuilder.h"
#include "LevelArt/PropProfileDataAsset.h"
#include "Components/SplineComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"

// Sets default values
ASplinePropActor::ASplinePropActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;
	
	Instances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Instances"));
	Instances->SetupAttachment(Spline);
}

void ASplinePropActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	Rebuild();
}

#if WITH_EDITOR
void ASplinePropActor::PostEditUndo()
{
	Super::PostEditUndo();

	// 언두는 스플라인만 되돌린다. 거기서 파생되는 메쉬는 우리가 다시 지어야 한다
	Rebuild();
}
#endif

void ASplinePropActor::Rebuild()
{
	ClearBuilt();
	
	if (!Profile || !Profile->IsUsable() || !Spline)
	{
		return;
	}
	
	// Repeat / Deform 분기
	switch (Profile->Mode)
	{
	case EPropPlacementMode::Repeat:
		BuildRepeat();
		break;
		
	case EPropPlacementMode::Deform:
		BuildDeform();
		break;
	}
}

void ASplinePropActor::ClearBuilt()
{
	if (Instances)
	{
		Instances->ClearInstances();
	}
	
	// Segments 배열은 핫 리로드·Undo 로 비워질 수 있다.
	// 배열을 믿지 말고 액터에 실제로 붙어 있는 것을 쓸어낸다
	TArray<USplineMeshComponent*> Found;
	GetComponents<USplineMeshComponent>(Found);
	
	for (USplineMeshComponent* Seg : Found)
	{
		if (Seg)
		{
			Seg->DestroyComponent();
		}
	}
	Segments.Empty();
}

void ASplinePropActor::CollectRepeatTransforms(TArray<FTransform>& OutTransforms) const
{
	const float Total = Spline->GetSplineLength();
	if (Total <= KINDA_SMALL_NUMBER)
	{
		return;
	}
	
	// 몇 개가 들어가는가. 스플라인이 간격보다 짧아도 하나는 놓는다.
	const int32 Count = FMath::Max(FMath::FloorToInt(Total / Profile->Spacing), 1);
	
	// 자투리 제거 - 간격을 길이에 맞춰 미세 조정해 끝점에 딱 맞춘다
	const float Step = Profile->bFitToSpline ? (Total / Count) : Profile->Spacing;
	
	// 닫힌 루프는 마지막 점이 첫 점과 겹치므로 하나 덜 놓는다
	const int32 Last = Spline->IsClosedLoop() ? Count - 1 : Count;
	
	FRandomStream Stream(Profile->Seed);
	const FQuat OffsetQuat = Profile->RotationOffset.Quaternion();
	
	OutTransforms.Reserve(Last + 1);
	
	for (int32 i = 0; i <= Last; ++i)
	{
		const float Dist = i * Step;
		
		// Local  좌표계로 뽑는다. World 로 뽑으면 액터를 옮길 때 프롭이 따라오지 않는다
		const FVector Location = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::Local);
		
		FRotator Rotation = FRotator::ZeroRotator;
		if (Profile->bAlignToSpline)
		{
			Rotation = Spline->GetRotationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::Local);
			
			if (Profile->bIgnoreSplineRoll)
			{
				Rotation.Roll = 0.0f;
			}
		}
		
		// 회전 합성은 쿼터니언으로. FRotator 끼리 더하면 축별 덧셈이라 결과가 다르다
		FQuat Quat = Rotation.Quaternion() * OffsetQuat;
		
		if (Profile->RotationJitter > 0.0f )
		{
			const float Yaw = Stream.FRandRange(-Profile->RotationJitter, Profile->RotationJitter);
			Quat = Quat * FQuat(FVector::UpVector, FMath::DegreesToRadians(Yaw));
		}
		
		FVector Scale = FVector::OneVector;
		if (Profile->ScaleJitter > 0.0f)
		{
			Scale *= 1.0f + Stream.FRandRange(-Profile->ScaleJitter, Profile->ScaleJitter);
		}
		
		OutTransforms.Emplace(Quat, Location, Scale);
	}
}

void ASplinePropActor::BuildRepeat()
{
	TArray<FTransform> Transforms;
	CollectRepeatTransforms(Transforms);
	
	PropMeshBuilder::BuildInstances(Instances, Profile, Transforms);
}

void ASplinePropActor::BuildDeform()
{
	if (Profile->TileLength > 0.f)
	{
		BuildDeformTiled();
	}
	else
	{
		BuildDeformPerPoint();
	}
}

void ASplinePropActor::BuildDeformTiled()
{
	const float Total = Spline->GetSplineLength();
	if (Total <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	// 자투리 제거 - 타일 길이를 스플라인 길이에 맞춰 미세 조정한다.
	// 끄면 TileLength 를 정확히 지키는 대신 마지막 조각이 짧게 남는다
	const int32 Count = FMath::Max(FMath::FloorToInt(Total / Profile->TileLength), 1);
	const float Step = Profile->bFitTiles ? (Total / Count) : Profile->TileLength;
	const int32 Num = Profile->bFitTiles ? Count : FMath::CeilToInt(Total / Step);

	Segments.Reserve(Num);

	for (int32 i = 0; i < Num; ++i)
	{
		const float D0 = i * Step;
		const float D1 = FMath::Min((i + 1) * Step, Total);

		const float Length = D1 - D0;
		if (Length <= KINDA_SMALL_NUMBER)
		{
			break;
		}

		const FVector StartPos = Spline->GetLocationAtDistanceAlongSpline(D0, ESplineCoordinateSpace::Local);
		const FVector EndPos   = Spline->GetLocationAtDistanceAlongSpline(D1, ESplineCoordinateSpace::Local);

		// 탄젠트 크기는 스플라인 전체 파라미터 기준이라 그대로 쓰면 조각 길이와 안 맞아
		// 메쉬가 부풀거나 뒤집힌다. 방향만 받아 조각 길이로 다시 재야 한다
		FVector StartTangent = Spline->GetDirectionAtDistanceAlongSpline(D0, ESplineCoordinateSpace::Local) * Length;
		FVector EndTangent   = Spline->GetDirectionAtDistanceAlongSpline(D1, ESplineCoordinateSpace::Local) * Length;

		if (Profile->SagAmount > 0.f)
		{
			const float Sag = Length * Profile->SagAmount;

			StartTangent.Z -= Sag;
			EndTangent.Z += Sag;
		}

		const FVector UpDir = Profile->bUseSplineUpVector
			? Spline->GetUpVectorAtDistanceAlongSpline(D0, ESplineCoordinateSpace::Local)
			: Profile->UpDirection;

		if (USplineMeshComponent* Seg = PropMeshBuilder::MakeSegment(this, Spline, Profile, StartPos, StartTangent, EndPos, EndTangent, UpDir))
		{
			Segments.Add(Seg);
		}
	}
}

void ASplinePropActor::BuildDeformPerPoint()
{
	const int32 NumPoints = Spline->GetNumberOfSplinePoints();
	if (NumPoints < 2)
	{
		return;
	}
	
	// 열린 스플라인은 점 N개에 구간 N-1개. 닫힌 루프는 마지막 점이 첫 점으로 돌아오므로 N개
	const int32 NumSegments = Spline->IsClosedLoop() ? NumPoints : NumPoints - 1;
	
	Segments.Reserve(NumSegments);
	
	for (int32 i = 0; i < NumSegments; ++i)
	{
		const int32 NextIndex = (i + 1) % NumPoints;
		
		// GetLocationAndTangentAtSplinePoint 은 점 위치에서 도함수를 평가하므로 Leave 탄젠트를 준다.
		// 구간의 끝에는 다음 점이 "도착하는" 방향(Arrive)이 들어가야 한다. 둘을 섞으면
		// 메쉬가 점을 지나쳐 휘었다가 돌아와 꺾여 보인다. 점을 추가해 Arrive 와 Leave 가
		// 갈라지는 순간부터 드러난다
		const FVector StartPos = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		const FVector EndPos   = Spline->GetLocationAtSplinePoint(NextIndex, ESplineCoordinateSpace::Local);

		FVector StartTangent = Spline->GetLeaveTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);
		FVector EndTangent   = Spline->GetArriveTangentAtSplinePoint(NextIndex, ESplineCoordinateSpace::Local);
		
		// 전선 처짐 - 스플라인 원본은 그대로 두고 넘길 사본만 기울인다
		// 원본을 고치면 OnConstruction 이 돌 때마다 처짐이 누적된다
		if (Profile->SagAmount > 0.f)
		{
			const float Sag = StartTangent.Size() * Profile->SagAmount;
			
			StartTangent.Z -= Sag;	// 아래로 향하며 출발
			EndTangent.Z += Sag;	// 아래에서 올라오며 도착
		}
		
		// 구간 시작점의 업 벡터를 쓴다. 스플라인이 점마다 계산해 주므로 구간끼리 이어지고
		// 롤도 반영된다. 끄면 프로파일의 고정값을 쓴다
		const FVector UpDir = Profile->bUseSplineUpVector
			? Spline->GetUpVectorAtSplinePoint(i, ESplineCoordinateSpace::Local)
			: Profile->UpDirection;

		if (USplineMeshComponent* Seg = PropMeshBuilder::MakeSegment(this, Spline, Profile, StartPos, StartTangent, EndPos, EndTangent, UpDir))
		{
			Segments.Add(Seg);
		}
	}
}
