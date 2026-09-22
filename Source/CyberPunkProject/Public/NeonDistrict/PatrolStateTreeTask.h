#pragma once

#include "CoreMinimal.h"
#include "StateTreePropertyRef.h"
#include "StateTreeTaskBase.h"
#include "StateTreePropertyRef.h"
#include "PatrolStateTreeTask.generated.h"

class APatrolRoute;

/*
 * Next Patrol Point 태스크의 인스턴스 데이터
 */
USTRUCT()
struct FStateTreeNextPatrolPointInstanceData
{
	GENERATED_BODY()
	
	// 순찰하는 액터. 컴포넌트에서 경로를 읽는다
	UPROPERTY(EditAnywhere, Category= Context)
	TObjectPtr<AActor> Actor;
	
	// 다음 순찰 지점. 트리의 TargetMovement_Location 파라미터에 바인딩
	UPROPERTY(EditAnywhere, Category= Out)
	TStateTreePropertyRef<FVector> NextLocation;
	
};

/*
 * 경로의 다음 지점을 골라 NextLocation에 쓴다. 경로가 없으면 실패
 */
USTRUCT(meta = (DisplayName = "Next Patrol Point", Category = "Neon District"))
struct FStateTreeNextPatrolPointTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	FStateTreeNextPatrolPointTask()
	{
		bShouldCallTick = false;
		bShouldStateChangeOnReselect = false;
	}
	
	using FInstanceDataType = FStateTreeNextPatrolPointInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

