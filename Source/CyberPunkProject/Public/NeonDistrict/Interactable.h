#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// 이 클래스는 손대지 않는다. 언리얼 리플렉션용 껍데기당
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

// 플레이어가 E 키로 상호작용할 수 있는 대상
class CYBERPUNKPROJECT_API IInteractable
{
	GENERATED_BODY()

public:
	//프롬포트에 표시할 문구 "문 열기", "키 줍기" 등
	virtual FText GetInteractionPrompt() const = 0;
	
	//지금 상호작용할 수 있는가. 잠긴 문이면 false
	virtual bool CanInteract(APawn* InteractingPawn) const { return true; }
	
	//실제 동작) 문이면 열리고, 키면 인벤토리에 들어간다
	virtual void Interact(APawn* InteractingPawn) = 0;
};