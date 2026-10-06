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
