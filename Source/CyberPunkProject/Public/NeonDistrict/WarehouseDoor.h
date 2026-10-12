// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NeonDistrict/Interactable.h"
#include "WarehouseDoor.generated.h"

class UStaticMeshComponent;

/*
 * 창고 문. 키를 가진 뒤에만 열린다. DL_Mission 레이어를 둔다
 */
UCLASS()
class CYBERPUNKPROJECT_API AWarehouseDoor : public AActor, public IInteractable
{
	GENERATED_BODY()
	
	// 경첩. 루트라서 액터 위치가 곧 회전축
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Hinge;
	
	// 문짝. 경첩에서 옆으로 밀어 둔다
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
	
protected:
	// 경첩에서 문짝을 밀어낼 거리. 폭의 절반만큼 밀어야 회전 중심이 문 가장자리에 온다.
	// 폭이 어느 축인지는 메시마다 다르므로 벡터로 받는다 (SM_RustboundDoor 는 X 가 폭)
	UPROPERTY(EditAnywhere, Category="Neon District")
	FVector HingeOffset = FVector(-38.6f, 0.f, 0.f);
	
	// 닫힌 자리에서 몇 도 더 도는가. 부호가 여는 방향이다 -
	// 플레이어가 서는 쪽을 쓸고 지나가지 않는 쪽으로 고른다
	// 열렸을 떄 경첩 Yaw
	UPROPERTY(EditAnywhere, Category="Neon District")
	float OpenAngle = 100.f;
	
	// 도/초
	UPROPERTY(EditAnywhere, Category="Neon District")
	float OpenSpeed = 180.f;
	
	UPROPERTY(VisibleInstanceOnly, Category="Neon District")
	bool bOpen = false;
	
public:	
	// Sets default values for this actor's properties
	AWarehouseDoor();
	
	//~ Begin AActor Interface
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	//~ End AActor Interface
	
	//~ Begin IInteractable Interface
	virtual FText GetInteractionPrompt() const override;
	virtual bool CanInteract(APawn* InteractingPawn) const override;
	virtual void Interact(APawn* InteractingPawn) override;
	//~ End IInteractable Interface
	
private:
	//키를 가진 단게인가
	bool HasKey() const;
	
	// 레벨에 놓인 각도. 여기서 OpenAngle 만큼 더 돈다
	float ClosedYaw = 0.f;
	
	// 지금 각도. 컴포넌트에서 되읽지 않고 우리가 센다
	float CurrentYaw = 0.f;
	
};
