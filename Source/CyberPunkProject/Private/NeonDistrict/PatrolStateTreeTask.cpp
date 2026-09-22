#include "NeonDistrict/PatrolStateTreeTask.h"
#include "NeonDistrict/MissionEnemyComponent.h"
#include "NeonDistrict/PatrolRoute.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeNextPatrolPointTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	if (!Data.Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Patrol] Actor 컨텍스트가 비어 있음"));
		return EStateTreeRunStatus::Failed;
	}
	
	// 컨텍스트 액터 -> 컴포넌트 -> 경로. 순찰 진행은 컴포넌트가 기억한다
	UMissionEnemyComponent* EnemyComponent = Data.Actor->FindComponentByClass<UMissionEnemyComponent>();
	const APatrolRoute* Route = EnemyComponent ? EnemyComponent->PatrolRoute.Get() : nullptr;
	
	if (!Route || Route->NumPoints() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Patrol] %s: 경로 없음(컴포넌트 %s)"), *Data.Actor->GetName(), EnemyComponent ? TEXT("있음") : TEXT("없음"));
		return EStateTreeRunStatus::Failed;
	}
	// 첫 진입이면 가장 가까운 점부터. 같은 경로를 도는 적들이 한 점에 물리지 않게
	if (EnemyComponent->PatrolDirection < 0)
	{
		EnemyComponent->PatrolIndex = Route->NearestIndex(Data.Actor->GetActorLocation());
	}
	else
	{
		EnemyComponent->PatrolIndex = Route->NextIndex(EnemyComponent->PatrolIndex, EnemyComponent->PatrolDirection);
	}
	const FVector Point = Route->GetPointWorld(EnemyComponent->PatrolIndex);
	
	// 바인딩된 곳(트리 파라미터)에 써 넣는다
	if (FVector* Target = Data.NextLocation.GetMutablePtr(Context))
	{
		*Target = Point;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Patrol] Next Location이 바인딩되지 않음"));
	}
	
	return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeNextPatrolPointTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
	return FText::FromString(TEXT("<b>Next Patrol Point</b>"));
}
#endif