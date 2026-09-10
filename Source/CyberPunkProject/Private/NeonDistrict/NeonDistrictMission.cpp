// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictMission.h"
#include "Engine/World.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"
#include "NeonDistrictGameMode.h"

// Sets default values
ANeonDistrictMission::ANeonDistrictMission()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	RestartPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RestartPoint"));
	RestartPoint->SetupAttachment(EntryTrigger);
	RestartPoint->SetRelativeLocation(FVector(-400.0f, 0.0f, 0.0f));
	
	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	SetRootComponent(EntryTrigger);
	
	EntryTrigger->SetBoxExtent(FVector(200.f, 200.f, 200.f));
	
	// 플레이어인지 확인 후 겹침 확인
	EntryTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	EntryTrigger->SetCollisionObjectType(ECC_WorldDynamic); 
	EntryTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	EntryTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	EntryTrigger->OnComponentBeginOverlap.AddDynamic(this, &ANeonDistrictMission::OnEntryOverlap);
}

void ANeonDistrictMission::AcceptMission()
{
	bAccepted = true;
	
	UE_LOG(LogTemp, Log, TEXT("[Mission] Accepted!"));
}

void ANeonDistrictMission::OnPlayerDied()
{
	// 다시 들어오면 새 시도로 카운트되도록 훈다 
	//@@@@@@@@@@@@@@@@@@@@@@@@@이거 분명 오류 
	//- 미션 시작될때 카운팅되는걸로 바꿔야함 
	//-칸 안에 들어올때마다 미션 시작 판정이면 안되고 처음에 들어오면 시작 이후로 죽었을때마다 카운팅 올라가고 다시시작 UI띄우도록해야함
	bRunning = false;
	
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 실패 - 재진입대기"));
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
	
	if (ANeonDistrictGameMode* GameMode = GetWorld()->GetAuthGameMode<ANeonDistrictGameMode>())
	{
		// 죽으면 여기로 돌아온다
		GameMode->SetCheckpoint(RestartPoint->GetComponentTransform());
		// 지금 진행 중인 미션이 나라고 알린다
		GameMode->SetActiveMission(this);
	}
	
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
