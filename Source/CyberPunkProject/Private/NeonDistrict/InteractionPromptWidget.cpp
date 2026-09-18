// Fill out your copyright notice in the Description page of Project Settings.


#include "NeonDistrict/InteractionPromptWidget.h"

#include "Interactable.h"
#include "InteractionComponent.h"

void UInteractionPromptWidget::BindToComponent(UInteractionComponent* Component)
{
	if (BoundComponent.IsValid())
	{
		BoundComponent->OnTargetChanged.RemoveAll(this);
	}
	
	BoundComponent = Component;
	if (!Component)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	Component->OnTargetChanged.AddUObject(this, &UInteractionPromptWidget::HandleTargetChanged);
	
	// 지금 이미 보고 있는 대상이 있을 수 있다
	HandleTargetChanged(Component->GetCurrentTarget());
}

void UInteractionPromptWidget::NativeDestruct()
{
	// 위젯이 사라질 때 구독을 끊는다
	if (BoundComponent.IsValid())
	{
		BoundComponent->OnTargetChanged.RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UInteractionPromptWidget::HandleTargetChanged(AActor* NewTarget)
{
	if (!NewTarget)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	FText Prompt = FText::GetEmpty();
	
	if (IInteractable* Interactable = Cast<IInteractable>(NewTarget))
	{
		Prompt = Interactable->GetInteractionPrompt();
	}
	
	BP_UpdatePrompt(Prompt);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
