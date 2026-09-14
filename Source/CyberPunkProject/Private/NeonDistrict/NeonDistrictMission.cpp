// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictMission.h"
#include "NeonDistrict/NeonDistrictCharacter.h"
#include "Engine/World.h"
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
	
	EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
	SetRootComponent(EntryTrigger);
	
	RestartPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("RestartPoint"));
	RestartPoint->SetupAttachment(EntryTrigger);
	RestartPoint->SetRelativeLocation(FVector(-400.0f, 0.0f, 0.0f));
	
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
	if (State == EMissionState::NotAccepted)
	{
		State = EMissionState::Accepted;
		
		UE_LOG(LogTemp, Log, TEXT("[Mission] Accepted!"));
	}
}

void ANeonDistrictMission::OnEntryOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 미션 수락 후 첫 진입에만 반응
	if (State != EMissionState::Accepted)
	{
		return;
	}
	
	APawn* Player = Cast<APawn>(OtherActor);
	if (!Player || !Player->IsPlayerControlled())
	{
		return;
	}
	
	StartMission(Player);
}

void ANeonDistrictMission::StartMission(APawn* Player)
{
	State = EMissionState::InProgress;
	DeathCount = 0;
	
	if (ANeonDistrictGameMode* GameMode = GetWorld()->GetAuthGameMode<ANeonDistrictGameMode>())
	{
		GameMode->SetCheckpoint(RestartPoint->GetComponentTransform());
		
		// 앞으로 새 Pawn이 생길 떄마다 다시 구독한다
		GameMode->OnPlayerPawnReady.AddUObject(this, &ANeonDistrictMission::BindToPlayer);
	}
	
	BindToPlayer(Player);
	SetupSegment();
	
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 시작"));
}

void ANeonDistrictMission::BindToPlayer(APawn* Player)
{
	if (ANeonDistrictCharacter* Character = Cast<ANeonDistrictCharacter>(Player))
	{
		Character->OnDied.AddUObject(this, &ANeonDistrictMission::HandlePlayerDied);
	}
}

void ANeonDistrictMission::HandlePlayerDied(ANeonDistrictCharacter* Character)
{
	if (State != EMissionState::InProgress) return;
	
	++DeathCount;
	UE_LOG(LogTemp,Warning, TEXT("[Mission] 사망 %d회 - 구간 재시작"), DeathCount);
	
	SetupSegment();
}

void ANeonDistrictMission::SetupSegment()
{
	//적 정리, 생성
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 구간 세팅"));
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
