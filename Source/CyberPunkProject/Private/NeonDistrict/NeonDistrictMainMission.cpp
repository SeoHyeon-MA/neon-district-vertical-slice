// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictMainMission.h"
#include "NeonDistrict/NeonDistrictCharacter.h"
#include "NeonDistrict/NeonDistrictGameMode.h"
#include "Components/ArrowComponent.h"

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
	//적 정리, 생성
	UE_LOG(LogTemp, Warning, TEXT("[Mission] 구간 세팅"));
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

