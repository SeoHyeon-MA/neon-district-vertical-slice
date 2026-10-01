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

	USplineMeshComponent* MakeSegment(AActor* Owner, USceneComponent* AttachTo, const UPropProfileDataAsset* Profile, const FVector& StartPos, const FVector& StartTangent, const FVector& EndPos, const FVector& EndTangent)
	{
		if (!Owner || !AttachTo || !Profile)
		{
			return nullptr;
		}
		
		// RF_Transient - 맵 파일에 저장되지 않게 한다. OnConstruction 이 어차피 매번 다시 만든다
		USplineMeshComponent* Seg = NewObject<USplineMeshComponent>(Owner, NAME_None, RF_Transient);
		
		// 등록 전에 해야 하는 것들. 등록 후에 부르면 경고만 나고 무시된다
		Seg->SetMobility(EComponentMobility::Movable);
		Seg->bSelectable = false;
		
		Seg->SetForwardAxis(Profile->ForwardAxis, /*bUpdateMesh*/ false);
		Seg->SetStaticMesh(Profile->Mesh);
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
