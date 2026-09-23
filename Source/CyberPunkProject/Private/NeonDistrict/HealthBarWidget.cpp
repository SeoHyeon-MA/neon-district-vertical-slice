// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/HealthBarWidget.h"
#include "Variant_Shooter/ShooterCharacter.h"

void UHealthBarWidget::BindToCharacter(AShooterCharacter* Character)
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnDamaged.RemoveDynamic(this, &UHealthBarWidget::HandleDamaged);
	}
	
	BoundCharacter = Character;
	if (!Character)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Character->OnDamaged.AddDynamic(this, &UHealthBarWidget::HandleDamaged);
	
	// 새 폰은 가득 찬 상태로 시작한다. 방송을 기다리지 않는다
	HandleDamaged(1.f);
}

void UHealthBarWidget::NativeDestruct()
{
	BindToCharacter(nullptr);
	Super::NativeDestruct();
}

void UHealthBarWidget::HandleDamaged(float LifePercent)
{
	// 죽으면 숨긴다. 부활하면 OnPossess 가 다시 채운다
	if (LifePercent <= 0.f)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	BP_UpdateHealthBar(LifePercent);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
