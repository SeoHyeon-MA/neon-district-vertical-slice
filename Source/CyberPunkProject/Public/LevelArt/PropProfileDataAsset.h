// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/DataAsset.h"
#include "PropProfileDataAsset.generated.h"

class UStaticMesh;

/** 스플라인 위에 메쉬를 올리는 두 가지 방식 (메쉬를 휘게 할 것이나 단순 반복 할 것이냐에 따라) */
UENUM(BlueprintType)
enum class EPropPlacementMode : uint8
{
	// 메쉬를 휘지 않고 일정 간격으로 복제한다. (펜스 기둥, 가로등, 파이프 브래킷)
	Repeat UMETA(DisplayName = "반복 배치 (펜스 기둥/가로등)"),
	// 메쉬를 스플라인 모양대로 휜다. (파이프 본페, 늘어진 전선, 펜스 그물망)
	Deform UMETA(DisplayName = "스플라인 변형 (파이프/전선)")
};

/**
 * ASplinePropActor 가 무엇을 어떻게 놓을지 담아두는 프로파일.
 * 액터에 메쉬를 직접 박지 않고 이 에셋만 교체하면
 * 펜스 -> 파이프 -> 전선으로 통째로 바뀐다.
 * 로직은 없고 값만 있으므로 .cpp 은 없다.
 */
UCLASS(BlueprintType)
class CYBERPUNKPROJECT_API UPropProfileDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// -- 메쉬 --
	
	// 스플라인을 따라 반복하거나 휠 메쉬
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Mesh")
	TObjectPtr<UStaticMesh> Mesh = nullptr;
	
	// 반복할지 휠지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Mesh")
	EPropPlacementMode Mode = EPropPlacementMode::Repeat;
	
	// 비워두면 메쉬 원본 메테리얼을 그대로 쓴다. 슬록 순서대로 덮어씌운다
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Mesh")
	TArray<TObjectPtr<UMaterialInterface>> MaterialOverrides;
	
	// -- 배치 --
	
	// 반복 배치일 때 메쉬 사이 거리(cm)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Placement", meta = (ClampMin = "1.0", UIMin = "50.0", UIMax = "1000.0", EditCondition = "Mode == EPropPlacementMode::Repeat", EditConditionHides))
	float Spacing = 200.0f;

	/**
	 *  간격을 스플라인 길이에 맞춰 미세 조정해서 끝에 자투리가 남지 않게 한다.
	 *  끄면 Spacing 을 정확히 지키는 대신 ㄷ마지막 구간이 어중간하게 비는 경우가 생긴다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Repeat", EditConditionHides))
	bool bFitToSpline = true;
	
	/** 메쉬를 스플라인 진행 방향으로 돌린다. 끄면 전부 같은 방향을 본다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Repeat", EditConditionHides))
	bool bAlignToSpline = true;
	
	/** 스플라인이 주는 롤(기울기)을 버리고 항상 수직으로 세운다. 기둥류는 켜두는 게 안전하다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Repeat && bAlignToSpline", EditConditionHides))
	bool bIgnoreSplineRoll = true;
	
	/** 메쉬가 엉뚱한 방향을 볼 때 보정하는 각도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement")
	FRotator RotationOffset = FRotator::ZeroRotator;
	
	/**
	 *  메쉬를 모델링할 때 어느 축이 "길이 방향"이었는지.
	 *  이게 틀리면 파이프가 납작하게 뭉게진다. 안 맞으면 Y, Z 로 돌려볼 것.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis = ESplineMeshAxis::X;
	
	/**
	 *	전선이 자기 무게로 쳐지는 정도. 0이면 직선 (무게가 없으므로)
	 *	구간 길이에 비례하므로 0.3이면 어느 간격에서나 비슷한 느낌으로 늘어진다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (ClampMin = "0.0", ClampMax = "2.0", EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	float SagAmount = 0.f;

	/**
	 *	구간을 이 길이(cm)로 쪼개 메쉬를 깐다. 0이면 쪼개지 않고 점 사이마다 메쉬 하나를 늘린다.
	 *	스플라인 메쉬는 반복이 아니라 변형이라, 쪼개지 않으면 점을 띄운 만큼 메쉬가 늘어난다.
	 *	값을 주면 점을 아무리 띄워도 메쉬 한 장의 길이가 일정하게 유지된다 (파이프, 펜스 그물망)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (ClampMin = "0.0", UIMax = "1000.0", EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	float TileLength = 0.f;

	/** 타일 길이를 스플라인 길이에 맞춰 미세 조정해 끝에 자투리가 남지 않게 한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Deform && TileLength > 0", EditConditionHides))
	bool bFitTiles = true;

	/**
	 *	메쉬의 "위"를 무엇으로 잡을지. 스플라인 메쉬는 업 벡터가 고정이라
	 *	구간이 수직에 가까워지면 축을 중심으로 돌아간다.
	 *	켜면 스플라인이 점마다 계산한 업 벡터를 써서 구간끼리 이어진다 (롤도 반영된다)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	bool bUseSplineUpVector = true;

	/** 위를 고정하고 싶을 때 쓸 방향(로컬). bUseSplineUpVector 를 끄면 이 값이 쓰인다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Placement", meta = (EditCondition = "Mode == EPropPlacementMode::Deform && !bUseSplineUpVector", EditConditionHides))
	FVector UpDirection = FVector::UpVector;

	// -- 크기 --

	/**
	 *	스태틱 메쉬를 통째로 키우거나 줄인다. 비율이 유지된다.
	 *
	 *	Repeat 은 각 메쉬에 그대로 곱한다.
	 *	Deform 은 길이 방향을 스플라인이 소유해서 배율을 직접 못 준다. 대신 단면과
	 *	TileLength 에 함께 곱해, 같은 메쉬가 더 큰 크기로 반복되게 만든다.
	 *	2로 두면 두 배 굵어지고 마디 간격도 두 배가 된다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Scale", meta = (ClampMin = "0.01", UIMin = "0.1", UIMax = "5.0"))
	float MeshScale = 1.f;

	/**
	 *	Deform 에서 진행 방향과 직각인 단면 배율. MeshScale 위에 곱해지는 미세 조정이다.
	 *	길이는 안 건드리므로 이것만 올리면 굵어지는 대신 납작해 보인다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Scale", meta = (ClampMin = "0.01", EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	FVector2D CrossSectionScale = FVector2D(1.f, 1.f);

	/**
	 *	스플라인 끝에서의 단면 배율. CrossSectionScale 과 다르게 두면 전체를 따라 가늘어진다.
	 *	타일로 쪼개도 전체 길이 기준으로 보간하므로 조각 경계에서 끊기지 않는다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Scale", meta = (ClampMin = "0.01", EditCondition = "Mode == EPropPlacementMode::Deform", EditConditionHides))
	FVector2D CrossSectionScaleEnd = FVector2D(1.f, 1.f);

	// -- 변화 주기 --
	
	/** 배치할 때마다 무작위로 더할 수 있는 회전 폭(도). 0 이면 전부 반듯하다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Variation", meta = (ClampMin = "0.0", ClampMax = "180.0", EditCondition = "Mode == EPropPlacementMode::Repeat", EditConditionHides))
	float RotationJitter = 0.0f;
	
	/** 크기를 흔드는 폭. 0.1이면 0.9 ~ 1.1 배 사이 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop|Variation", meta = (ClampMin = "0.0", ClampMax = "0.9", EditCondition = "Mode == EPropPlacementMode::Repeat", EditConditionHides))
	float ScaleJitter = 0.0f;
	
	/**
	 *	난수 시드. 고정해두지 않으면 스플라인을 건드릴 때마다
	 *	흔들림과 색이 전부 바뀌어서 작업을 할 수 없다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Variation")
	int32 Seed = 12345;
	
	// -- 렌더링 --
	
	/** 끄면 그림자 패스 비용이 통째로 빠진다. (전선, 잔가지 프롭) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Rendering")
	bool bCastShadow = true;
	
	/** 장식용 파이프나 전선은 충돌이 필요 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Rendering")
	bool bEnableCollision = true;
	
	/** 이 거리(cm)를 넘으면 그리지 않는다. 0 이면 제한 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop|Rendering", meta = (ClampMin = "0.0", UIMax = "20000.0"))
	float CullDistance = 0.0f;
	
public:
	// -- 공개 API -- 
	
	/** 이 프로파일로 실제로 무언가를 그릴 수 있는 상태인지 */
	bool IsUsable() const { return Mesh != nullptr; }
};
