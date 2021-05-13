// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MortalCryGameMode.generated.h"

UCLASS(minimalapi)
class AMortalCryGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMortalCryGameMode();
	
	UFUNCTION(BlueprintCallable)
	void SwapPlayers();

	virtual void StartPlay() override;
};



