// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "NotificationSubsystem.generated.h"

// 화면에 띄울 짧은 문장
DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotification, const FText&);

/**
 *  한 줄짜리 알림을 흘려보내는 통로. 보내는 쪽은 받는 쪽을 모른다
 */
UCLASS()
class CYBERPUNKPROJECT_API UNotificationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	FOnNotification OnNotification;
	
	// 알림을 흘린다
	void Push(const FText& Message);
	
	// 월드를 들고 있지 않은 곳에서 한 줄로 부르기 위한 헬퍼
	static void Notify(const UObject* WorldContext, const FText& Message);
};
