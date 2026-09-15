// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class IInteractable;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnInteractionTargetChanged, AActor*/*NewTarget, 없으면 nullptr*/)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERPUNKPROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionComponent();
	
	// 대상이 바뀔 때 방송, 프롬포트 UI가 구독한다
	FOnInteractionTargetChanged OnTargetChanged;

	// E키 입력이 여기로 온다
	void TryInteract();
	
	// 지금 바라보는 대상, 없으면 nullptr
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }
	
protected:
	// 탐색 거리
	UPROPERTY(EditAnywhere, Category="Interaction", meta=(ClampMin=0, ClampMax=1000, Units="cm"))
	float TraceDistance = 200.f;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// 앞을 훑어서 대상을 갱신
	void UpdateTarget();
	
	TWeakObjectPtr<AActor> CurrentTarget;
		
};
