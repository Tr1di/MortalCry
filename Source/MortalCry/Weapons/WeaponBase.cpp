// Fill out your copyright notice in the Description page of Project Settings.

#include "WeaponBase.h"

AWeaponBase::AWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;

	AActor::SetReplicateMovement(true);
	
	//GetSkeletalMeshComponent()->SetIsReplicated(true);
	
	InteractText = "<Name>";
}

void AWeaponBase::Sheath_Implementation()
{
	Execute_EndAttack(this);
	Execute_EndAttack(this);
	Execute_EndAlterAttack(this);
	Execute_EndAlterAction(this);
}

void AWeaponBase::Interact_Implementation(AActor* InInstigator)
{
	SetOwner(InInstigator);
	GetSkeletalMeshComponent()->SetSimulatePhysics(false);
	GetSkeletalMeshComponent()->MoveIgnoreActors.Add(GetOwner());
	//GetSkeletalMeshComponent()->SetCollisionProfileName("NoCollision");
}

void AWeaponBase::EndInteract_Implementation()
{
	AActor* OldOwner = GetOwner();
	SetOwner(nullptr);
	GetSkeletalMeshComponent()->SetSimulatePhysics(true);
	GetSkeletalMeshComponent()->MoveIgnoreActors.Remove(OldOwner);
	//GetSkeletalMeshComponent()->SetCollisionProfileName("BlockAll");
}

void AWeaponBase::GetDescription_Implementation(FString& OutString) const
{
	OutString = InteractText
	.Replace(TEXT("<Name>"), *Execute_GetName(this))
	.Replace(TEXT("\n"), *FString("\n"));
}
