// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/WeaponBase.h"

#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundCue.h"

AWeaponBase::AWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	GetMeshTP()->SetOwnerNoSee(true);
	GetMeshTP()->SetSimulatePhysics(true);
	
	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>("MeshFP");
	MeshFP->SetupAttachment(GetSkeletalMeshComponent());
	MeshFP->SetSimulatePhysics(false);
	MeshFP->SetOnlyOwnerSee(true);
	MeshFP->SetCastShadow(false);
	MeshFP->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	bReplicates = true;
	AActor::SetReplicateMovement(true);
}

float AWeaponBase::PlayWeaponAnimation(const FWeaponAnim& Animation)
{
	float Duration = 0.0f;
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.PawnFP : Animation.PawnTP;
		if (UseAnim)
		{
			Duration = MyPawn->PlayAnimMontage(UseAnim);
		}
	}

	return Duration;
}

void AWeaponBase::StopWeaponAnimation(const FWeaponAnim& Animation)
{
	if (MyPawn)
	{
		UAnimMontage* UseAnim = MyPawn->IsFirstPerson() ? Animation.PawnFP : Animation.PawnTP;
		if (UseAnim)
		{
			MyPawn->StopAnimMontage(UseAnim);
		}
	}
}

UAudioComponent* AWeaponBase::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = nullptr;
	if (Sound && MyPawn)
	{
		AC = UGameplayStatics::SpawnSoundAttached(Sound, MyPawn->GetRootComponent());
	}

	return AC;
}

void AWeaponBase::AttachToPawn()
{
	if (MyPawn)
	{
		// Remove and hide both first and third person meshes
		DetachFromPawn();

		// For locally controller players we attach both weapons and let the bOnlyOwnerSee, bOwnerNoSee flags deal with visibility.
		FName AttachPoint = TEXT("GripPoint");
		FName FPAttachPoint = TEXT("GripPointFP");
		
		if( MyPawn->IsLocallyControlled() )
		{
			USkeletalMeshComponent* PawnMesh1p = MyPawn->GetSpecificPawnMesh(true);
			USkeletalMeshComponent* PawnMesh3p = MyPawn->GetSpecificPawnMesh(false);
			GetMeshFP()->AttachToComponent(PawnMesh1p, FAttachmentTransformRules::KeepRelativeTransform, FPAttachPoint);
			GetMeshTP()->AttachToComponent(PawnMesh3p, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
		else
		{
			USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
			USkeletalMeshComponent* UsePawnMesh = MyPawn->GetPawnMesh();
			UseWeaponMesh->AttachToComponent(UsePawnMesh, FAttachmentTransformRules::KeepRelativeTransform, AttachPoint);
		}
	}
}

void AWeaponBase::DetachFromPawn()
{
	GetMeshFP()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	GetMeshTP()->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
}

void AWeaponBase::OnUnEquip()
{
}

void AWeaponBase::Interact_Implementation(AActor* InInstigator)
{
	SetOwner(InInstigator);
	MyPawn = Cast<AMortalCryCharacter>(InInstigator);
	GetMeshTP()->SetSimulatePhysics(false);
	GetMeshTP()->SetCollisionProfileName("NoCollision");
	//AttachToPawn();
}

void AWeaponBase::StopInteracting_Implementation()
{
	SetOwner(nullptr);
	MyPawn = nullptr;
	GetMeshTP()->SetSimulatePhysics(true);
	GetMeshTP()->SetCollisionProfileName("BlockAll");
	DetachFromPawn();
}

FText AWeaponBase::GetDescription_Implementation() const
{
	return Description;
}

USkeletalMeshComponent* AWeaponBase::GetWeaponMesh() const
{
	return GetMyPawn()->IsFirstPerson() ? GetMeshFP() : GetMeshTP();
}