// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplinePropActor.generated.h"

class USplineMeshComponent;
class UPropProfileDataAsset;
class UInstancedStaticMeshComponent;
class USplineComponent;
/**
 *	스플라인을 따라 Prop을 1)반복하거나 2)휘어서 배치하는 액터.
 *	레벨에 놓고 점을 찍은 뒤 Profile 만 바꾸면
 *	펜스 -> 파이프 -> 전선으로 통째로 바뀐다.
 *	
 *	에디터 전용 도구 액터. 게임플레이 로직이 X, Tick 사용하지 X
 */
UCLASS()
class CYBERPUNKPROJECT_API ASplinePropActor : public AActor
{
	GENERATED_BODY()
	
	/** 사용자가 점을 찍는 경로. 루트다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> Spline;
	
	/** Repeat 모드에서 쓰는 인스턴스 통. 수백 개여도 드로우콜 1회 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> Instances;
	
protected:
	/** 무엇을 어떻게 놓을지. 이것만 바꾸면 종류가 통째로 바뀐다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SplineProp")
	TObjectPtr<UPropProfileDataAsset> Profile;
	
	/**
	 *	Deform 모드에서 구간마다 만든 컴포넌트.
	 *	UPROPERTY 가 없으면 GC 가 수거해서 크래시한다.
	 *	Transient 인 이유는 OnConstruction 이 매번 다시 만들기 때문이다 - 맵에 저장할 이유가 없다.
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<USplineMeshComponent>> Segments;
	
public:	
	// Sets default values for this actor's properties
	ASplinePropActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif
	
public:
	/** 지금 상태로 다시 짓는다. OnConstruction 이 부르고, 밖에서 강제로 부를 수도 있다 */
	UFUNCTION(CallInEditor, Category="SplineProp")
	void Rebuild();
	
protected:
	/** 이전에 만든 것을 전부 치운다. Rebuild 의 첫 줄 */
	void ClearBuilt();
	
	/** 스플라인 위 트랜스폼만 계산한다. 렌더링을 모른다 */
	void CollectRepeatTransforms(TArray<FTransform>& OutTransforms) const;
	
	/** 계산된 트래스폼을 인스턴스로 올린다 */
	void BuildRepeat();
	
	/** 구간마다 SplineMeshComponent 를 만들어 메쉬를 휜다. TileLength 가 있으면 쪼개서 깐다 */
	void BuildDeform();

	/** 점 사이마다 메쉬 하나. 점 간격이 곧 메쉬 길이다 */
	void BuildDeformPerPoint();

	/** 스플라인 길이를 TileLength 로 나눠 깐다. 점 간격과 무관하게 길이가 일정하다 */
	void BuildDeformTiled();
};
