// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MortalCryMovementComponent.h"

#include "Character/MortalCryCharacter.h"
#include "Net/UnrealNetwork.h"

UMortalCryMovementComponent::UMortalCryMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SlowWalkSpeedModifier = 0.5f;
	RunSpeedModifier = 1.5f;

	bWantsToWalk = false;
	bWantsToRun = false;
}

void UMortalCryMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMortalCryMovementComponent, bWantsToWalk);
	DOREPLIFETIME(UMortalCryMovementComponent, bWantsToRun);
}

void UMortalCryMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UMortalCryMovementComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
	
	if ( IsRunning() )
	{
		SetRunning(false);
	}
}

void UMortalCryMovementComponent::SetWalk(bool bNewWalking)
{
	bWantsToWalk = bNewWalking;
	
	if ( GetOwnerRole() < ROLE_Authority )
	{
		ServerSetWalking(bNewWalking);
	}
}

void UMortalCryMovementComponent::SetRunning(bool bNewRunning)
{
	bWantsToRun = bNewRunning;

	if ( GetOwnerRole() < ROLE_Authority )
	{
		ServerSetRunning(bNewRunning);
	}
}

bool UMortalCryMovementComponent::ServerSetWalking_Validate(bool bNewWalking)
{
	return true;
}

void UMortalCryMovementComponent::ServerSetWalking_Implementation(bool bNewWalking)
{
	SetWalk(bNewWalking);
}

bool UMortalCryMovementComponent::ServerSetRunning_Validate(bool bNewRunning)
{
	return true;
}

void UMortalCryMovementComponent::ServerSetRunning_Implementation(bool bNewRunning)
{
	SetRunning(bNewRunning);
}

void UMortalCryMovementComponent::OnStartWalking()
{
	if ( IsRunning() )
	{
		SetRunning(false);
	}

	SetWalk(true);
}

void UMortalCryMovementComponent::OnStopWalking()
{
	SetWalk(false);
}

void UMortalCryMovementComponent::OnStartRunning()
{
	if ( IsWalking() )
	{
		SetWalk(false);
	}
	
	SetRunning(true);
}

void UMortalCryMovementComponent::OnStopRunning()
{
	SetRunning(false);
}

float UMortalCryMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	switch ( MovementMode )
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		if ( IsSlowWalking() )	MaxSpeed *= SlowWalkSpeedModifier;
		if ( IsRunning() )		MaxSpeed *= RunSpeedModifier;
	default:
		break;
	}
	
	AMortalCryCharacter* Character = Cast<AMortalCryCharacter>(CharacterOwner);
	if (Character)
	{
		if (!Character->IsAlive())
		{
			MaxSpeed *= 0.1f;
		}
		if ( Character->IsTargeting() )
		{
			MaxSpeed *= 0.5;
		}
	}
	
	return MaxSpeed;
}

bool UMortalCryMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && !IsRunning();
}

bool UMortalCryMovementComponent::IsSlowWalking() const
{
	if (!CharacterOwner)
	{
		return false;
	}
	return bWantsToWalk && !Velocity.IsZero();
}

bool UMortalCryMovementComponent::IsRunning() const
{
	if (!CharacterOwner)
	{
		return false;
	}
	return bWantsToRun && !Velocity.IsZero() && (Velocity.GetSafeNormal2D() | CharacterOwner->GetActorForwardVector()) > 0.5f;
}
