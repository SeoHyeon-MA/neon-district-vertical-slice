// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelArt/PropMeshBuilder.h"

#include "LevelArt/PropProfileDataAsset.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SplineMeshComponent.h"

namespace PropMeshBuilder
{
	void BuildInstances(UInstancedStaticMeshComponent* ISM, const UPropProfileDataAsset* Profile, const TArray<FTransform>& Transforms)
	{
		if (!ISM || !Profile || Transforms.Num() == 0) return;
	
		ISM->SetStaticMesh(Profile->Mesh);
		ApplyRenderingSettings(ISM, Profile);
		
		ISM->AddInstances(Transforms, /*bShouldReturnIndices*/ false);
	}

	USplineMeshComponent* MakeSegment(AActor* Owner, USceneComponent* AttachTo, const UPropProfileDataAsset* Profile, const FVector& StartPos, const FVector& StartTangent, const FVector& EndPos, const FVector& EndTangent, const FVector& UpDir)
	{
		if (!Owner || !AttachTo || !Profile)
		{
			return nullptr;
		}
		
		// RF_Transient - 맵 파일에 저장되지 않게 한다. OnConstruction 이 어차피 매번 다시 만든다
		USplineMeshComponent* Seg = NewObject<USplineMeshComponent>(Owner, NAME_None, RF_Transient);
		
		// 등록 전에 해야 하는 것들. 등록 후에 부르면 경고만 나고 무시된다
		Seg->SetMobility(EComponentMobility::Movable);

		// 끄면 히트 프록시가 안 생겨 클릭이 메쉬를 통과한다. 그러면 스플라인 액터를
		// 고르려면 아웃라이너나 스플라인 점을 찾아야 한다.
		// 켜 두면 뷰포트에서 메쉬를 찍는 것으로 소유 액터가 선택된다.
		// 구간은 RF_Transient 라 OnConstruction 마다 새로 만들어지지만,
		// 레벨 에디터는 컴포넌트가 아니라 액터를 고르므로 선택이 끊기지 않는다
		Seg->bSelectable = true;
		
		Seg->SetForwardAxis(Profile->ForwardAxis, /*bUpdateMesh*/ false);
		Seg->SetStaticMesh(Profile->Mesh);

		// 업 벡터가 진행 방향과 나란해지면 메쉬가 축을 중심으로 돌아간다.
		// 그때는 기본값을 쓰는 수밖에 없고, 아니면 받은 값을 정규화해 넣는다
		const FVector Dir = (EndPos - StartPos).GetSafeNormal();
		FVector SafeUp = UpDir.GetSafeNormal();
		if (SafeUp.IsNearlyZero() || FMath::Abs(FVector::DotProduct(SafeUp, Dir)) > 0.99f)
		{
			SafeUp = FVector::UpVector;
		}
		Seg->SetSplineUpDir(SafeUp, /*bUpdateMesh*/ false);

		// 마지막에 부르며 메쉬를 갱신한다
		Seg->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
		
		ApplyRenderingSettings(Seg, Profile);
		
		Seg->AttachToComponent(AttachTo, FAttachmentTransformRules::KeepRelativeTransform);
		
		// 없으면 컴포넌트는 만들어졌는데 렌더 씬에 안 들어간다
		Seg->RegisterComponent();
		
		return Seg;
	}

	void ApplyRenderingSettings(UPrimitiveComponent* Component, const UPropProfileDataAsset* Profile)
	{
		if (!Component || !Profile) return;
		
		// 그림자 패스는 배경 프로에서 가장 큰 비용 / 전선,잔가지는 꺼도 티가 안나므로 끈다
		Component->SetCastShadow(Profile->bCastShadow);
		
		Component->SetCollisionEnabled(Profile->bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
		
		// 0 이면 무제한. 그대로 넘기면 "0m 밖은 안 그림" 이 되어서 전부 사라진다
		if (Profile->CullDistance > 0.f)
		{
			Component->SetCullDistance(Profile->CullDistance);
		}
		
		for (int32 i = 0; i < Profile->MaterialOverrides.Num(); ++i)
		{
			if (UMaterialInterface* Mat = Profile->MaterialOverrides[i])
			{
				Component->SetMaterial(i, Mat);
			}
		}
	}
}
