// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/HitFeedbackWidget.h"
#include "Variant_Shooter/ShooterCharacter.h"

void UHitFeedbackWidget::BindToCharacter(AShooterCharacter* Character)
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnDamaged.RemoveDynamic(this, &UHitFeedbackWidget::HandleDamaged);
	}
	
	BoundCharacter = Character;
	
	// 새 폰은 가득 찬 상태로 시작한다
	LastLiftPercent = 1.f;
	
	if (Character)
	{
		Character->OnDamaged.AddDynamic(this, &UHitFeedbackWidget::HandleDamaged);
	}
}

void UHitFeedbackWidget::NativeDestruct()
{
	BindToCharacter(nullptr);
	Super::NativeDestruct();
}

void UHitFeedbackWidget::HandleDamaged(float LifePercent)
{
	// 체력이 줄었을 때만 번쩍인다. 부활 시 1.0 방송에는 반응하지 않는다
	if (LifePercent < LastLiftPercent - KINDA_SMALL_NUMBER)
	{
		BP_OnHit(LifePercent);
	}
	
	LastLiftPercent = LifePercent;
}

