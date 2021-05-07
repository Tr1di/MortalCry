// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryGameMode.h"
#include "MortalCryHUD.h"
#include "MortalCryCharacter.h"
#include "TeamSettings.h"
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

void AMortalCryGameMode::SwapPlayers()
{
	TArray<APlayerController*> Controllers;
	
	for ( auto Controller = GetWorld()->GetPlayerControllerIterator(); Controller; ++Controller)
	{
		Controllers.Add(Controller->Get());
	}

	Controllers.RemoveAll([](APlayerController* C) { return !C; });
	
	if ( Controllers.Num() > 1 )
	{
		SwapPlayerControllers(Controllers[0],
							SpawnPlayerController(ENetRole::ROLE_Authority, ""));
	}
}

void AMortalCryGameMode::StartPlay()
{
	Super::StartPlay();
	FGenericTeamId::SetAttitudeSolver(&UTeamSettings::GetAttitude);
}
