// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/DeathScreenWidget.h"
#include "NeonDistrict/NeonDistrictCharacter.h"

void UDeathScreenWidget::BindToCharacter(ANeonDistrictCharacter* Character)
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnDied.RemoveAll(this);
	}
	
	BoundCharacter = Character;
	
	if (Character)
	{
		Character->OnDied.AddUObject(this, &UDeathScreenWidget::HandleDied);
		
		// 새 폰을 받았다 = 재시작이 끝났다. 화면을 걷는다
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDeathScreenWidget::NativeDestruct()
{
	BindToCharacter(nullptr);
	Super::NativeDestruct();
}

void UDeathScreenWidget::HandleDied(ANeonDistrictCharacter* Character)
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	BP_OnDeath(Character ? Character->GetRestartDelay() : 0.f);
}
