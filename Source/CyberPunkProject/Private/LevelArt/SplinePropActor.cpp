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
		
		FVector StartPos, StartTangent, EndPos, EndTangent;
		Spline->GetLocationAndTangentAtSplinePoint(i, StartPos, StartTangent, ESplineCoordinateSpace::Local);
		Spline->GetLocationAndTangentAtSplinePoint(NextIndex, EndPos, EndTangent, ESplineCoordinateSpace::Local);
		
		if (USplineMeshComponent* Seg = PropMeshBuilder::MakeSegment(this, Spline, Profile, StartPos, StartTangent, EndPos, EndTangent))
		{
			Segments.Add(Seg);
		}
		
		// RF_Transient - 맵 파일에 저장되지 않게 한다 OnConstruction 이 어차피 매번 다시 만든다
		USplineMeshComponent * Seg = NewObject<USplineMeshComponent>(this, NAME_None, RF_Transient);
		
		// 등록 전에 해야 한다. 등록 후에 부르면 경고만 나고 무시된다
		Seg->SetMobility(EComponentMobility::Movable);
		Seg->bSelectable = false;
		
		Seg->SetForwardAxis(Profile->ForwardAxis, /*bUpdateMesh*/ false);
		Seg->SetStaticMesh(Profile->Mesh);
		Seg->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		
		Seg->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
		
		// 없으면 컴포넌트는 만들어졌는데 렌더 씬에 안 들어감. 화면에 아무것도 안 보인다
		Seg->RegisterComponent();
		
		Segments.Add(Seg);
	}
}
