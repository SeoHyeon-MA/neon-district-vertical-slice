// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NeonDistrict/MissionDialogueTypes.h"
#include "DialogueWidget.generated.h"

class UDataTable;

DECLARE_DELEGATE_OneParam(FOnDialogueEffect, EDialogueEffect);
DECLARE_DELEGATE(FOnDialogueFinished);

/**
 * 대사를 한 줄씩 보여준다. 미션은 모르고, 줄의 Effect(미션상태)를 델리게이트로 넘길 뿐이다.
 */
UCLASS(Abstract)
class CYBERPUNKPROJECT_API UDialogueWidget  : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 줄이 끝날 때 Effect가 None이 아니면, NPC가 묶는다
	FOnDialogueEffect OnEffect;
	
	// 마지막 줄을 넘겼을 때, 컨트롤러가 묶는다
	FOnDialogueFinished OnFinished;
	
	bool IsActive() const { return !CurrentRow.IsNone(); }
	
	// 시작 행부터 보여준다
	void Start(UDataTable* InTable, FName StartRow);
	
	// 다음 줄로. 없으면 끝낸다
	void Advance();
	
protected:
	// 블루프린트가 화자.본문 텍스트 블록을 갱신
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "Update Line"))
	void BP_UpdateLine(const FText& Speaker, bool bIsPlayer);
	
	// 본문. 타이핑 중에는 잘린 문자열, 완성되면 전체
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "Set Line Text"))
	void BP_SetLineText(const FText& VisibleText);
	
	// 줄이 다 찍혔는가. "[E] 다음" 안내를 여기에 맞춘다
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta = (DisplayName = "Set Line Complete"))
	void BP_SetLineComplete(bool bComplete);
	
	// 한 글자당 시간
	UPROPERTY(EditDefaultsOnly, Category="Neon District", meta = (ClampMin = 0.005, ClampMax = 0.2, Units = "s"))
	float CharInterval = 0.03f;
	
private:
	// 행을 찾아 화면에 올린다. 없으면 끝낸다
	void ShowRow(FName Row);
	
	void Finish();
	
	/* 대사 타이핑 연출 */
	// 한 글자 더 보인다. 타이머가 반복 호출
	void TypeNextChar();
	
	// 타이핑을 끝내고 전체를 보여준다
	void CompleteLine();
	
	bool IsTyping() const {return VisibleChars < FullLine.Len(); }
	
	// 지금 줄의 전체 본문
	FString FullLine;
	
	int32 VisibleChars = 0;
	
	FTimerHandle TypeTimer;
	
	UPROPERTY()
	TObjectPtr<UDataTable> Table;
	
	FName CurrentRow;
	FName NextRow;
	EDialogueEffect CurrentEffect = EDialogueEffect::None;
};
