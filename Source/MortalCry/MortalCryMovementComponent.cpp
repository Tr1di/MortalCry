// Fill out your copyright notice in the Description page of Project Settings.


#include "MortalCryMovementComponent.h"

#include "MortalCryCharacter.h"

float UMortalCryMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	// TODO: А сюда добавишь код :)
	
	AMortalCryCharacter* Character = Cast<AMortalCryCharacter>(GetOwner());
	
	if (Character)
	{
		if (!Character->IsAlive())
		{
			MaxSpeed *= 0.3f;
		}
	}
	
	return MaxSpeed;
}
