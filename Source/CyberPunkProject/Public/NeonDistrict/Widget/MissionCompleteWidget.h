// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionCompleteWidget.generated.h"

class UMissionRegistry;
class ANeonDistrictMission;
/**
 *  메인 미션이 끝나면 랭크와 사망 횟수를 보여준다
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UMissionCompleteWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 블루프린트가 결과를 채운다
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName="ShowResult"))
	void BP_ShowResult(const FText& Rank, int32 DeathCount);
	
	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UUserWidget Interface
	
private:
	// 등록부에서 미션이 빠질 때. 완료로 빠진 메인 미션만 본다
	void HandleActiveMissionChanged(ANeonDistrictMission* Mission, bool bRegistered);
	
	TWeakObjectPtr<UMissionRegistry> BoundRegistry;
};
