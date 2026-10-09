// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PropProfileDataAsset.h"

class AActor;
class USplineMeshComponent;
class UPropProfileDataAsset;
/**
 *	[그래픽 층] 계산된 배치 결과를 실제 컴포넌트로 올린다.
 *	
 *	스플라인 수학은 여기 없다. 들어오는 것은 이미 계산이 끝난 트랜스폼과 탄젠트고
 *	이 층은 그것을 어떻게 그릴지 (인스턴싱이냐 스플라인 메쉬냐, 그림자·LOD·컬링은 어떻게)만 결정한다.
 *	상태가 없으므로 UCLASS 가 아니라 자유 함수다
 */
namespace PropMeshBuilder
{
	/** Repeat - 트랜스폼 배열을 인스턴스 통에 올린다 */
	void BuildInstances(UInstancedStaticMeshComponent* ISM, const UPropProfileDataAsset* Profile, const TArray<FTransform>& Transforms);
	
	/** Deform - 한 구간을 만들어 등록까지 마치고 돌려준다. UpDir 은 로컬 공간이다 */
	USplineMeshComponent* MakeSegment(AActor* Owner, USceneComponent* AttachTo, const UPropProfileDataAsset* Profile, const FVector& StartPos, const FVector& StartTangent, const FVector& EndPos, const FVector& EndTangent, const FVector& UpDir);
	
	/** 두 모드가 공유하는 렌더링 설정. 머테리얼 덮어쓰기 포함 */
	void ApplyRenderingSettings(UPrimitiveComponent* Component, const UPropProfileDataAsset* Profile);
}
