// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HealthComponent.h"

UHealthComponent::UHealthComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHealth = Health = 1000.f;
}

void UHealthComponent::Update(float HealthChange)
{
	Health += HealthChange;
	Health = FMath::Clamp(Health, 0.f, MaxHealth);
}

float UHealthComponent::GetHealth() const
{
	return Health / MaxHealth;
}

bool UHealthComponent::IsAlive() const
{
	return Health > 0.f;
}

