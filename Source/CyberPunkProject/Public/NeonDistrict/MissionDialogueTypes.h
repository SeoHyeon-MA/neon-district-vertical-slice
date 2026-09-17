#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MissionDialogueTypes.generated.h"

// 대사 줄이 끝날 때 미션에 일으키는 일
UENUM()
enum class EDialogueEffect : uint8
{
	None,
	AcceptMission,
	CompleteMission
};

// 대사 한 줄, 데이터 테이블의 행
USTRUCT(BlueprintType)
struct FDialogueLine : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category= "Dialogue")
	FText Speaker;
	
	UPROPERTY(EditAnywhere, Category= "Dialogue", meta = (MultiLine = true))
	FText Text;
	
	// 다음 줄의 행 이름. 비어 있으면 대화가 끝난다
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName NextRow;
	
	// 이 줄이 끝날 때 적용
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	EDialogueEffect Effect = EDialogueEffect::None;
};