// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryGameMode.h"
#include "MortalCryHUD.h"
#include "MortalCryCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMortalCryGameMode::AMortalCryGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMortalCryHUD::StaticClass();
}
