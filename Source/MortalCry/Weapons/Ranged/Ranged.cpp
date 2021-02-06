// Fill out your copyright notice in the Description page of Project Settings.


#include "Ranged.h"

#include "Kismet/GameplayStatics.h"
#include "MortalCry/MortalCryCharacter.h"
#include "MortalCry/MortalCryProjectile.h"

ARanged::ARanged(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(GetSkeletalMeshComponent());
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));
}

void ARanged::Attack_Implementation()
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (ProjectileClass)
		{
			UWorld* const World = GetWorld();
			if (World)
			{
				FRotator SpawnRotation = FP_MuzzleLocation->GetComponentRotation();
				SpawnRotation.Yaw = Character->GetActorRotation().Yaw;
                // MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
                const FVector SpawnLocation = (FP_MuzzleLocation ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) /*+ SpawnRotation.RotateVector(GunOffset)*/;

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				if (World->SpawnActor<AMortalCryProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams))
				{
					if (FireSound)
					{
						UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
					}
					
					if (FireAnimation)
					{
						Character->PlayAnimMontage(FireAnimation, 1.f);
					}
				}
			}
		}
	}
}

void ARanged::EndAttack_Implementation()
{
	
}
