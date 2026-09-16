#pragma once

#include "CoreMinimal.h"
#include "MissionTypes.generated.h"

// 미션 분류. 메인은 한 번에 하나, 사이드는 여러 개가 병행된다
UENUM()
enum class EMissionCategory : uint8 
{
	Main,
	Side
};

