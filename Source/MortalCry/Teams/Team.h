#pragma once

#include "CoreMinimal.h"
#include "Team.generated.h"

UENUM(BlueprintType)
namespace ETeam 
{
	enum Type
	{
		Neutral	UMETA(DisplayName = "Neutral"),
		Traitor	UMETA(DisplayName = "Traitor"),
		Player	UMETA(DisplayName = "Player"),
		Bot		UMETA(DisplayName = "Bot"),
		Boss	UMETA(DisplayName = "Boss"),
		MAX		UMETA(Hidden)
	};
}
