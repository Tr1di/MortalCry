// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon.h"

AWeapon::AWeapon(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;

	AActor::SetReplicateMovement(true);
	
	//GetSkeletalMeshComponent()->SetIsReplicated(true);
	
	InteractText = "<Name>";
}

void AWeapon::Interact_Implementation(AActor* InInstigator)
{
	SetOwner(InInstigator);
	GetSkeletalMeshComponent()->SetSimulatePhysics(false);
	GetSkeletalMeshComponent()->MoveIgnoreActors.Add(GetOwner());
	//GetSkeletalMeshComponent()->SetCollisionProfileName("NoCollision");
}

void AWeapon::EndInteract_Implementation(AActor* InInstigator)
{
	AActor* OldOwner = GetOwner();
	SetOwner(nullptr);
	GetSkeletalMeshComponent()->SetSimulatePhysics(true);
	GetSkeletalMeshComponent()->MoveIgnoreActors.Remove(OldOwner);
	//GetSkeletalMeshComponent()->SetCollisionProfileName("BlockAll");
}

void AWeapon::GetDescription_Implementation(FString& OutString) const
{
	OutString = InteractText
	.Replace(TEXT("<Name>"), *Execute_GetName(this))
	.Replace(TEXT("\n"), *FString("\n"));
}
