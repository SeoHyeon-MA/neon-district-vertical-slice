#include "NeonDistrict/DialogueWidget.h"
#include "Engine/DataTable.h"

void UDialogueWidget::Start(UDataTable* InTable, FName StartRow)
{
	Table = InTable;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ShowRow(StartRow);
}

void UDialogueWidget::Advance()
{
	if (!IsActive()) return;
	
	// 찍히는 중이면 줄만 완성하고 넘기지 않는다.
	// 한 번 더 눌러야 다음 줄 - 실수로 대사를 건너뛰지 않게
	if (IsTyping())
	{
		CompleteLine();
		return;
	}
	
	if (CurrentEffect != EDialogueEffect::None)
	{
		OnEffect.ExecuteIfBound(CurrentEffect);
	}
	ShowRow(NextRow);
}

void UDialogueWidget::ShowRow(FName Row)
{
	const FDialogueLine* Line = (Table && !Row.IsNone()) ? Table->FindRow<FDialogueLine>(Row, TEXT("DialogueWidget")) : nullptr;
	if (!Line)
	{
		Finish();
		return;
	}
	
	CurrentRow = Row;
	NextRow = Line->NextRow;
	CurrentEffect = Line->Effect;
	BP_UpdateLine(Line->Speaker, Line->bIsPlayer);  
	
	FullLine = Line->Text.ToString();
	VisibleChars = 0;
	
	BP_SetLineText(FText::GetEmpty());
	BP_SetLineComplete(false);
	
	if (FullLine.IsEmpty())
	{
		CompleteLine();
		return;
	}
	
	GetWorld()->GetTimerManager().SetTimer(TypeTimer, this, &UDialogueWidget::TypeNextChar, CharInterval, true);
}

void UDialogueWidget::Finish()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TypeTimer);
	}
	
	CurrentRow = NAME_None;
	NextRow = NAME_None;
	CurrentEffect = EDialogueEffect::None;
	FullLine.Reset();
	VisibleChars = 0;
	
	SetVisibility(ESlateVisibility::Collapsed);
	OnFinished.ExecuteIfBound();
}

void UDialogueWidget::TypeNextChar()
{
	++VisibleChars;
	
	if (!IsTyping())
	{
		CompleteLine();
		return;
	}
	
	BP_SetLineText(FText::FromString(FullLine.Left(VisibleChars)));
}

void UDialogueWidget::CompleteLine()
{
	GetWorld()->GetTimerManager().ClearTimer(TypeTimer);
	
	VisibleChars = FullLine.Len();
	
	BP_SetLineText(FText::FromString(FullLine));
	BP_SetLineComplete(true);
}
