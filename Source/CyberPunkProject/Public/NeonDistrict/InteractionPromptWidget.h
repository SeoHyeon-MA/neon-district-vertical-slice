// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptWidget.generated.h"

class UInteractionComponent;

/**
 * 
 */
UCLASS()
class CYBERPUNKPROJECT_API UInteractionPromptWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	//컴포넌트의 방송을 구독한다
	void BindToComponent(UInteractionComponent* Component);
	
protected:
	// 블루프린트가 텍스트 블록을 갱신
	UFUNCTION(BlueprintImplementableEvent, Category="Neon District", meta=(DisplayName="Update Prompt"))
	void BP_UpdatePrompt(const FText& Prompt);
	
	virtual void NativeDestruct() override;
	
private:
	//대상이 바뀌면 불린다
	void HandleTargetChanged(AActor* NewTarget);
	
	TWeakObjectPtr<UInteractionComponent> BoundComponent;
};
