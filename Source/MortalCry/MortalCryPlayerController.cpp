// Fill out your copyright notice in the Description page of Project Settings.


#include "MortalCryPlayerController.h"

#include "GameFramework/GameModeBase.h"

AMortalCryPlayerController::AMortalCryPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
}

void AMortalCryPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMortalCryPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

AActor* AMortalCryPlayerController::Trace(TSubclassOf<UInterface> SearchClass)
{
	if ( OnTrace.IsBound() )
	{
		return OnTrace.Execute(SearchClass);
	}

	return nullptr;
}