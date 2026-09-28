// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/AmmoCounterWidget.h"

#include "ShooterCharacter.h"

void UAmmoCounterWidget::BindToCharacter(AShooterCharacter* Character)
{
	if (BoundCharacter.IsValid())
	{
		BoundCharacter->OnBulletCountUpdated.RemoveDynamic(this, &UAmmoCounterWidget::HandleBulletCountUpdated);
	}
	
	BoundCharacter = Character;
	if (!Character)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	Character->OnBulletCountUpdated.AddDynamic(this, &UAmmoCounterWidget::HandleBulletCountUpdated);
	
	// 무기를 들기 전에는 방송이 오지 않는다. 일단 비워 둔다
	HandleBulletCountUpdated(0, 0);
}

void UAmmoCounterWidget::NativeDestruct()
{
	BindToCharacter(nullptr);
	Super::NativeDestruct();
}

void UAmmoCounterWidget::HandleBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// 무기가 없으면 숨긴다
	if (MagazineSize <= 0)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	// 자릿수를 고정해 숫자가 흔들리지 않게 한다. (007 / 030)
	const int32 Digits = FString::FromInt(MagazineSize).Len();
	
	FNumberFormattingOptions Format;
	Format.MinimumIntegralDigits = Digits;
	Format.UseGrouping = false;
	
	const bool bLowAmmo = Bullets <= FMath::CeilToInt(MagazineSize * LowAmmoRatio);
	
	BP_UpdateAmmo(FText::AsNumber(Bullets, &Format), FText::AsNumber(MagazineSize,&Format), bLowAmmo);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
