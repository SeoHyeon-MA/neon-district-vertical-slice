// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/Widget/NotificationWidget.h"

#include "NotificationSubsystem.h"

void UNotificationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 평소에도 떠 있고, 투명도로만 보였다 사라진다. 클릭은 막지 않는다
	SetVisibility(ESlateVisibility::HitTestInvisible);
	
	UNotificationSubsystem* Subsystem = GetWorld()->GetSubsystem<UNotificationSubsystem>();
	if (!Subsystem) return;
	
	BoundSubsystem = Subsystem;
	Subsystem->OnNotification.AddUObject(this, &UNotificationWidget::HandleNotification);
}

void UNotificationWidget::NativeDestruct()
{
	if (BoundSubsystem.IsValid())
	{
		BoundSubsystem->OnNotification.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UNotificationWidget::HandleNotification(const FText& Message)
{
	BP_ShowNotification(Message);
}
