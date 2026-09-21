// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/WarehouseShutter.h"
#include "NeonDistrict/WarehouseMission.h"
#include "Components/StaticMeshComponent.h"
#include "WarehouseMission.h"
#include "Tests/AutomationEditorCommon.h"

// Sets default values
AWarehouseShutter::AWarehouseShutter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	// 벽이다. 기본 BlockAll
}

// Called when the game starts or when spawned
void AWarehouseShutter::BeginPlay()
{
	Super::BeginPlay();
	
	// 레이어가 올라올 때 미션은 이미 진행 중이다
	AWarehouseMission* Mission = AWarehouseMission::FindActive(this);
	if (!Mission)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Shutter] 진행 중인 창고 미션이 없어 구독하지 않음"));
		return;
	}
	
	BoundMission = Mission;
	Mission->OnStateChanged.AddUObject(this, &AWarehouseShutter::HandleMissionStateChanged);
	
	//구독 전에 이미 그 단계 일 수 있다.
	HandleMissionStateChanged(Mission);
}

void AWarehouseShutter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundMission.IsValid())
	{
		BoundMission->OnStateChanged.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void AWarehouseShutter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	FVector Loc = Mesh->GetRelativeLocation();
	Loc.Z = FMath::FInterpConstantTo(Loc.Z, OpenHeight, DeltaSeconds, OpenSpeed);
	Mesh->SetRelativeLocation(Loc);
	
	if (FMath::IsNearlyEqual(Loc.Z, OpenHeight))
	{
		SetActorTickEnabled(false);
	}
}

void AWarehouseShutter::HandleMissionStateChanged(ANeonDistrictMission* Mission)
{
	if (bOpen || !BoundMission.IsValid()) return;
	
	if (BoundMission->GetStep() >= EWarehouseStep::ItemAcquired)
	{
		Open();
	}
}

void AWarehouseShutter::Open()
{
	bOpen = true;
	SetActorTickEnabled(true);
	UE_LOG(LogTemp, Warning, TEXT("[Shutter] 개방"));
}

