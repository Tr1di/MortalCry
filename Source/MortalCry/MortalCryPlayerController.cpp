// Fill out your copyright notice in the Description page of Project Settings.


#include "MortalCryPlayerController.h"

#include "Possessive.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

AMortalCryPlayerController::AMortalCryPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	
	OnNewPawn.AddLambda([&](APawn* NewPawn)
	{
		if ( !MainPawn && NewPawn )
		{
			MainPawn = NewPawn;
		}
	});
}

void AMortalCryPlayerController::Tick(float DeltaSeconds) {}

void AMortalCryPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	//InputComponent->BindAction("Interact", IE_DoubleClick, this, &AMortalCryPlayerController::DoPossess);
	InputComponent->BindAction("PossessMain", IE_DoubleClick, this, &AMortalCryPlayerController::DoUnPossess);
}

APawn* AMortalCryPlayerController::Trace_Implementation()
{
	if ( GetPawn() && GetPawn()->ActorHasTag("Aga") )
	{
		return nullptr;
	}
	
	if ( APawn* ControlledPawn = GetPawn() )
	{
		FHitResult OutHit;
		const FVector Start = ControlledPawn->GetPawnViewLocation();
		const FVector ForwardVector = GetControlRotation().Vector();
		const FVector End = Start + ForwardVector * 1000.f;

		TArray<AActor*> ActorsToIgnore({ ControlledPawn });
		ControlledPawn->GetAttachedActors(ActorsToIgnore, false);
		
		if ( UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, TraceTypeQuery5, true, ActorsToIgnore,
                                                    EDrawDebugTrace::None, OutHit, true) )
		{
			if ( APawn* HPawn = Cast<APawn>(OutHit.Actor) )
			{
				if ( !HPawn->IsPlayerControlled() && HPawn->Implements<UPossessive>() && IPossessive::Execute_IsPossessive(HPawn) )
				{
					return HPawn;
				}
			}
		}
	}
	return nullptr;
}

void AMortalCryPlayerController::Interact()
{
	GetWorldTimerManager().SetTimer(InteractTimer, this, &AMortalCryPlayerController::Interact_Internal, 0.5f);
}

void AMortalCryPlayerController::Interact_Internal()
{
	GetWorldTimerManager().ClearTimer(InteractTimer);
	OnInteract.ExecuteIfBound();
}

void AMortalCryPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	OnInteract.Clear();
	OnEndInteract.Clear();
}

void AMortalCryPlayerController::Interact_Released()
{
	if ( GetWorldTimerManager().IsTimerActive(InteractTimer) || !OnInteract.IsBound() )
	{
		GetWorldTimerManager().ClearTimer(InteractTimer);
		//DoPossess();
	}
	else
	{
		OnEndInteract.ExecuteIfBound();
	}
}

void AMortalCryPlayerController::DoUnPossess_Implementation()
{
	if ( GetPawn() && GetPawn()->ActorHasTag("Aga") )
	{
		return;
	}
	
	if ( GetPawn() != MainPawn )
	{
		if ( GetPawn() && GetOwner() )
		{
			GetPawn()->SetOwner(GetOwner());
			SetOwner(nullptr);
		}
		Possess(MainPawn);
	}
}