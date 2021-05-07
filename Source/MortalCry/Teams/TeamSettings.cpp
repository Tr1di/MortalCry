// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamSettings.h"

UTeamSettings::UTeamSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	typedef ETeamAttitude::Type EAttitude;
	
	TeamAttitudes =
	{
		// Neutral				// Traitor				// Player				// Bot					// Boss
		{ EAttitude::Friendly,	EAttitude::Hostile, 	EAttitude::Neutral, 	EAttitude::Neutral, 	EAttitude::Neutral  }, 	// Neutral
		{ EAttitude::Hostile, 	EAttitude::Friendly, 	EAttitude::Hostile, 	EAttitude::Hostile, 	EAttitude::Hostile  },	// Traitor
		{ EAttitude::Neutral, 	EAttitude::Hostile, 	EAttitude::Friendly,	EAttitude::Hostile,		EAttitude::Hostile  },	// Player
		{ EAttitude::Neutral, 	EAttitude::Hostile, 	EAttitude::Hostile, 	EAttitude::Friendly,	EAttitude::Friendly },	// Bot
		{ EAttitude::Neutral, 	EAttitude::Hostile, 	EAttitude::Hostile, 	EAttitude::Neutral, 	EAttitude::Friendly }	// Boss
	};
}

const UTeamSettings* UTeamSettings::Get()
{
	return GetDefault<UTeamSettings>();
}

ETeamAttitude::Type UTeamSettings::GetAttitude(const FGenericTeamId Of, const FGenericTeamId Towards)
{
	auto& TeamAttitudes = Get()->TeamAttitudes;

	UE_LOG(LogTemp, Warning, TEXT("Check attitude of %i to %i"), Of.GetId(), Towards.GetId());
	
	if ( TeamAttitudes.IsValidIndex(Of.GetId()) && TeamAttitudes.IsValidIndex(Towards.GetId()) )
	{
		auto& Attitudes = TeamAttitudes[Of.GetId()].Attitude;
		if ( Attitudes.IsValidIndex(Towards.GetId()) )
		{
			return Attitudes[Towards.GetId()];
		}
	}
	
	return ETeamAttitude::Neutral;
}
