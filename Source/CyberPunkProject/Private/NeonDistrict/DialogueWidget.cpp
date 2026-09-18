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
	
	// 줄이 끝났다 - Effect를 먼저 넘기고 다음 줄로
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
	BP_UpdateLine(Line->Speaker, Line->Text);
}

void UDialogueWidget::Finish()
{
	CurrentRow = NAME_None;
	NextRow = NAME_None;
	CurrentEffect = EDialogueEffect::None;
	SetVisibility(ESlateVisibility::Collapsed);
	OnFinished.ExecuteIfBound();
}
