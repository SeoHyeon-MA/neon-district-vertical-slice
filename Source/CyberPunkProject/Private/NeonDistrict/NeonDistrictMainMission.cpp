// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictMainMission.h"
#include "NeonDistrict/NeonDistrictCharacter.h"
#include "NeonDistrict/NeonDistrictGameMode.h"
#include "Components/ArrowComponent.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/WorldPartitionLevelStreamingDynamic.h"
#include "WorldPartition/WorldPartitionRuntimeCell.h"
#include "TimerManager.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "UObject/Package.h"

ANeonDistrictMainMission::ANeonDistrictMainMission()
{
	Category = EMissionCategory::Main;
	
	RestartPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RestartPoint"));
	RestartPoint->SetupAttachment(GetRootComponent());
	RestartPoint->SetRelativeLocation(FVector(-400.f, 0.f, 0.f));
}

FText ANeonDistrictMainMission::GetRank() const
{
	if (DeathCount == 0) return FText::FromString(TEXT("S"));
	if (DeathCount == 1) return FText::FromString(TEXT("A"));
	if (DeathCount == 2) return FText::FromString(TEXT("B"));
	return FText::FromString(TEXT("C"));
}

void ANeonDistrictMainMission::BindToPlayer(APawn* Player)
{
	if (ANeonDistrictCharacter* Character = Cast<ANeonDistrictCharacter>(Player))
	{
		Character->OnDied.AddUObject(this, &ANeonDistrictMainMission::HandlePlayerDied);
	}
}

void ANeonDistrictMainMission::HandlePlayerDied(ANeonDistrictCharacter* Character)
{
	if (State != EMissionState::InProgress) return;
	
	++DeathCount;
	UE_LOG(LogTemp,Warning, TEXT("[Mission] 사망 %d회 - 구간 재시작"), DeathCount);
	
	SetupSegment();
	NotifyStateChanged();
}

void ANeonDistrictMainMission::SetupSegment()
{
	UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(this);
	if (!Manager || !SegmentLayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Mission] 구간 레이어가 없어 초기화를 건너뜀"));
		return;
	}

	// 레이어 셀의 레벨 패키지 이름을 기억해 둔다. 내려간 스트리밍 레벨은 월드 목록에서 빠져 나중엔 찾을 수 없다
	SegmentCellPackages.Reset();
	for (ULevelStreaming* StreamingLevel : GetWorld()->GetStreamingLevels())
	{
		const UWorldPartitionLevelStreamingDynamic* CellLevel = Cast<UWorldPartitionLevelStreamingDynamic>(StreamingLevel);
		const UWorldPartitionRuntimeCell* Cell = CellLevel ? CellLevel->GetWorldPartitionRuntimeCell() : nullptr;
		if (Cell && Cell->ContainsDataLayer(SegmentLayer))
		{
			SegmentCellPackages.Add(CellLevel->GetWorldAssetPackageFName());
		}
	}

	// 1단계: 내린다. 같은 프레임에 올리면 스트리밍이 마지막 상태만 보므로, 내려간 뒤에 올린다
	Manager->SetDataLayerRuntimeState(SegmentLayer, EDataLayerRuntimeState::Unloaded);
	GetWorldTimerManager().SetTimer(SegmentReloadTimer, this, &ANeonDistrictMainMission::ActiveSegmentWhenUnloaded, 0.1f, true);
}

void ANeonDistrictMainMission::ActiveSegmentWhenUnloaded()
{
	// 이 레이어의 셀이 아직 월드에 붙어 있으면 내려가는 중. 다음 틱에 다시
	for (ULevelStreaming* StreamingLevel : GetWorld()->GetStreamingLevels())
	{
		const UWorldPartitionLevelStreamingDynamic* CellLevel = Cast<UWorldPartitionLevelStreamingDynamic>(StreamingLevel);
		if (!CellLevel || !CellLevel->GetLoadedLevel()) continue;

		const UWorldPartitionRuntimeCell* Cell = CellLevel->GetWorldPartitionRuntimeCell();
		if (Cell && Cell->ContainsDataLayer(SegmentLayer)) return;
	}

	// 내려간 레벨이 메모리에 남아 있으면 엔진이 그 레벨을 그대로 재사용한다 (World Partition은 GC를 미룬다).
	// 새로 읽게 하려면 GC로 지운 뒤에 올려야 한다
	for (const FName& PackageName : SegmentCellPackages)
	{
		if (StaticFindObjectFast(UPackage::StaticClass(), nullptr, PackageName, EFindObjectFlags::None, RF_NoFlags, EInternalObjectFlags::Garbage))
		{
			GEngine->ForceGarbageCollection(true);
			return;
		}
	}

	GetWorldTimerManager().ClearTimer(SegmentReloadTimer);

	// 2단계: 올린다. 레이어 안의 액터가 에디터 저장 상태로 다시 생긴다
	if (UDataLayerManager* Manager = UDataLayerManager::GetDataLayerManager(this))
	{
		Manager->SetDataLayerRuntimeState(SegmentLayer, EDataLayerRuntimeState::Activated);
		UE_LOG(LogTemp, Warning, TEXT("[Mission] 구간 레이어 재활성화"));
	}
}

void ANeonDistrictMainMission::StartMission(APawn* Player)
{
	DeathCount = 0;
	
	if (ANeonDistrictGameMode* GameMode = GetWorld()->GetAuthGameMode<ANeonDistrictGameMode>())
	{
		GameMode->SetCheckpoint(RestartPoint->GetComponentTransform());
		GameMode->OnPlayerPawnReady.AddUObject(this, &ANeonDistrictMainMission::BindToPlayer);
	}
	
	BindToPlayer(Player);
	SetupSegment();
	
	// 상태 전환, 등록, 방송은 부모가
	Super::StartMission(Player);
}


