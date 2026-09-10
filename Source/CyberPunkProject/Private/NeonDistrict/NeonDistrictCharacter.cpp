// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NeonDistrictCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Controller.h"
#include "Engine/DamageEvents.h"


void ANeonDistrictCharacter::Die()
{
	//무기 비활성화, 이동 정지, 충돌 해제, 입력 차단 (부모 기능)
	Super::Die();
	
	//부모가 예약한 5초 리스폰을 취소
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	
	//대신 잠시 뒤 레벨을 다시 시작
	GetWorldTimerManager().SetTimer(RestartTimer, this, &ANeonDistrictCharacter::RequestRestart, RestartDelay, false);
}

void ANeonDistrictCharacter::RequestRestart()
{
	// Destroy() 뒤에는 GetController()가 nullptr이므로 미리 잡아둔다
	AController* MyController = GetController();
	UWorld* World = GetWorld();
	
	//캐릭터만 파괴한다. 구간 리셋은 미션 시스템이 생기면 게임모드가 맡는다.
	Destroy();
	
	if (World && MyController)
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			GameMode->RestartPlayer(MyController);
		}
	}
}

void ANeonDistrictCharacter::NDKill()
{
	TakeDamage(MaxHP * 2.0f, FDamageEvent(), nullptr, this);
}
