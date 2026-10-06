// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/WarehouseDoor.h"
#include "NeonDistrict/WarehouseMission.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWarehouseDoor::AWarehouseDoor()
{
 	// 열리는 동안만 틱. 평소엔 꺼 둔다
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	SetRootComponent(Hinge);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Hinge);
	Mesh->SetRelativeLocation(FVector(0.f, 50.f, 0.f)); // 문 폭의 절반. 메시에 맞춰 조정
	//문은 벽이다. 기본 blockAll - 트레이스도 맞고 플레이어도 막는다.
}

void AWarehouseDoor::BeginPlay()
{
	Super::BeginPlay();
	
	// 레벨 배치 회전을 기준으로 삼는다. 문을 어떻게 돌려 놔도 OpenAngle 은 "열리는 양"이 된다.
	// Hinge 가 루트라 이 값이 곧 액터의 배치 Yaw 다
	ClosedYaw = Hinge->GetRelativeRotation().Yaw;
	CurrentYaw = ClosedYaw;
}

// Called every frame
void AWarehouseDoor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	const float TargetYaw = ClosedYaw + OpenAngle;
	
	// 각도를 우리가 센다. GetRelativeRotation 은 정규화된 값을 주므로
	// -190 이 +170 으로 돌아와, 그걸 되읽어 쫓으면 문이 반대로 한 바퀴 돈다
	CurrentYaw = FMath::FInterpConstantTo(CurrentYaw, TargetYaw, DeltaSeconds, OpenSpeed);
	
	FRotator Rot = Hinge->GetRelativeRotation();
	Rot.Yaw = CurrentYaw;
	Hinge->SetRelativeRotation(Rot);
	
	if (FMath::IsNearlyEqual(CurrentYaw, TargetYaw))
	{
		SetActorTickEnabled(false);
	}
}

FText AWarehouseDoor::GetInteractionPrompt() const
{
	if (bOpen)		return FText::GetEmpty();
	if (HasKey())	return FText::FromString(TEXT("창고 열기"));
	return FText::FromString(TEXT("잠겨 있다 - 창고 키 필요"));
}

bool AWarehouseDoor::CanInteract(APawn* InteractingPawn) const
{
	return !bOpen && HasKey();
}


void AWarehouseDoor::Interact(APawn* InteractingPawn)
{
	bOpen = true;
	SetActorTickEnabled(true);
}

bool AWarehouseDoor::HasKey() const
{
	const AWarehouseMission* Mission = AWarehouseMission::FindActive(this);
	return Mission && Mission->GetStep() >= EWarehouseStep::KeyAcquired;
}
