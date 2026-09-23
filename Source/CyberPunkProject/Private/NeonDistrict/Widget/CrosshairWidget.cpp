// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/CrosshairWidget.h"

#include "NeonDistrictCharacter.h"

void UCrosshairWidget::BindToCharacter(ANeonDistrictCharacter* Character)
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnAimingChanged.RemoveAll(this);
	}
	
	BoundCharacter = Character;
	if (!Character)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Character->OnAimingChanged.AddUObject(this, &UCrosshairWidget::HandleAimingChanged);
	
	// 이미 조준 중일 수 있다
	HandleAimingChanged(Character->IsAiming());
}

void UCrosshairWidget::NativeDestruct()
{
	BindToCharacter(nullptr);
	Super::NativeDestruct();
}

void UCrosshairWidget::HandleAimingChanged(bool bAiming)
{
	BP_SetAiming(bAiming);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

