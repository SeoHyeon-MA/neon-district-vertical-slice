// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictMission.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "Components/BoxComponent.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"

// Sets default values
ANeonDistrictMission::ANeonDistrictMission()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	SetRootComponent(EntryTrigger);
	
	EntryTrigger->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	
	// Trigger는 언리얼이 트리거 볼륨용으로 미리 만들어둔 프리셋
	EntryTrigger->SetCollisionProfileName(TEXT("Trigger"));
	
	EntryTrigger->OnComponentBeginOverlap.AddDynamic(this, &ANeonDistrictMission::OnEntryOverlap);
}

void ANeonDistrictMission::AcceptMission()
{
	bAccepted = true;
	
	UE_LOG(LogTemp, Log, TEXT("[Mission] Accepted!"));
}

void ANeonDistrictMission::OnEntryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 아직 미션을 안 받았다면 리턴
	if (!bAccepted || bRunning) return;
	
	// 플레이어가 조종하는 폰인가
	APawn* Player = Cast<APawn>(OtherActor);
	if (!Player || !Player->IsPlayerControlled()) return;
	
	StartMission(Player);
}

void ANeonDistrictMission::StartMission(APawn* Player)
{
	bRunning = true;
	++AttemptCount;
	
	UE_LOG(LogTemp, Warning, TEXT("[Mission] Started! - AttemptCount : %d"), AttemptCount);
}

//############################디버그 설정##############
#if !UE_BUILD_SHIPPING

/** 콘솔에서 부르는 실제 함수 */
static void NDAcceptMissions(UWorld* World)
{
	if (!World)
	{
		return;
	}

	int32 Count = 0;

	for (TActorIterator<ANeonDistrictMission> It(World); It; ++It)
	{
		It->AcceptMission();
		++Count;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Mission] %d개 수락됨"), Count);
}

/** 콘솔 명령 등록. 이 전역 변수가 만들어질 때 등록된다 */
static FAutoConsoleCommandWithWorld GNDAcceptMissionCmd(
	TEXT("ND.AcceptMission"),
	TEXT("배치된 미션을 모두 수락 상태로 만든다 (디버그용)"),
	FConsoleCommandWithWorldDelegate::CreateStatic(&NDAcceptMissions));

#endif
