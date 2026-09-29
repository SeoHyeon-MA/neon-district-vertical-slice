// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/NotificationSubsystem.h"

void UNotificationSubsystem::Push(const FText& Message)
{
	// 빈 문장은 흘리지 않는다. 메세지를 안 채운 픽업이 빈 줄을 띄우지 않게
	if (Message.IsEmpty()) return;
	
	UE_LOG(LogTemp, Log, TEXT("[Notification] %s"), *Message.ToString());
	
	OnNotification.Broadcast(Message);
}

void UNotificationSubsystem::Notify(const UObject* WorldContext, const FText& Message)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World) return;
	
	if (UNotificationSubsystem* Subsystem = World->GetSubsystem<UNotificationSubsystem>())
	{
		Subsystem->Push(Message);
	}
}
