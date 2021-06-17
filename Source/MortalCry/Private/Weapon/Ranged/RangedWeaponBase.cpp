// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Ranged/RangedWeaponBase.h"

#include "AIController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Player/MortalCryPlayerController.h"

typedef ERangedWeaponState::Type EWeaponState;

ARangedWeaponBase::ARangedWeaponBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bWantsToFire = false;
	bPendingReload = false;
	AmmoInClip = 0;
	BurstCounter = 0;
	CurrentState = EWeaponState::Idle;
}

void ARangedWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARangedWeaponBase, AmmoInClip);
}

bool ARangedWeaponBase::ServerStartFire_Validate()
{
	return true;
}

void ARangedWeaponBase::ServerStartFire_Implementation()
{
	StartFire();
}

bool ARangedWeaponBase::ServerStopFire_Validate()
{
	return true;
}

void ARangedWeaponBase::ServerStopFire_Implementation()
{
	StopFire();
}

void ARangedWeaponBase::DetermineWeaponState()
{
	EWeaponState NewState = EWeaponState::Idle;

	if( bPendingReload  )
    {
    	if( !CanReload() )
    	{
    		NewState = CurrentState;
    	}
    	else
    	{
    		NewState = EWeaponState::Reloading;
    	}
    }		
    else if ( !bPendingReload && bWantsToFire && CanFire() )
    {
    	NewState = EWeaponState::Firing;
    }

	SetWeaponState(NewState);
}

void ARangedWeaponBase::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{
		OnBurstFinished();
	}

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{
		OnBurstStarted();
	}
}

void ARangedWeaponBase::OnBurstStarted()
{
	HandleFiring();
}

void ARangedWeaponBase::OnBurstFinished()
{
	StopSimulatingWeaponFire();
	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
}

void ARangedWeaponBase::UseAmmo()
{
	if ( !HasInfiniteClip() )
	{
		AmmoInClip--;
	}
}

void ARangedWeaponBase::HandleReFiring()
{
	HandleFiring();
}

bool ARangedWeaponBase::ServerHandleFiring_Validate()
{
	return true;
}

void ARangedWeaponBase::ServerHandleFiring_Implementation()
{
	const bool bShouldUpdateAmmo = AmmoInClip > 0 && CanFire();

	HandleFiring();

	if (bShouldUpdateAmmo)
	{
		UseAmmo();

		BurstCounter++;
	}
}

void ARangedWeaponBase::SimulateWeaponFire()
{
	if (GetLocalRole() == ROLE_Authority && CurrentState != EWeaponState::Firing)
	{
		return;
	}

	if ( MuzzleFX )
	{
		USkeletalMeshComponent* UseWeaponMesh = GetWeaponMesh();
		if ( !bLoopedMuzzleFX || !MuzzlePSC )
		{
			// Split screen requires we create 2 effects. One that we see and one that the other player sees.
			if( GetMyPawn() && GetMyPawn()->IsLocallyControlled() )
			{
				AController* PlayerCon = GetMyPawn()->GetController();				
				if( PlayerCon )
				{
					//GetMeshFP()->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, GetMeshFP(), MuzzleAttachPoint);
					MuzzlePSC->bOwnerNoSee = false;
					MuzzlePSC->bOnlyOwnerSee = true;

					//GetMeshTP()->GetSocketLocation(MuzzleAttachPoint);
					MuzzlePSCSecondary = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, GetMeshTP(), MuzzleAttachPoint);
					MuzzlePSCSecondary->bOwnerNoSee = true;
					MuzzlePSCSecondary->bOnlyOwnerSee = false;				
				}				
			}
			else
			{
				MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, UseWeaponMesh, MuzzleAttachPoint);
			}
		}
	}

	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayWeaponAnimation(FireAnim);
		bPlayingFireAnim = true;
	}

	if (bLoopedFireSound)
	{
		if (FireAC)
		{
			FireAC = PlayWeaponSound(FireLoopSound);
		}
	}
	else
	{
		PlayWeaponSound(FireSound);
	}

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetMuzzleLocation(), 1, GetMyPawn(), 0, TEXT("Shoot"));
	
	APlayerController* PC = GetMyPawn() ? Cast<APlayerController>(GetMyPawn()->Controller) : nullptr;
	if ( PC && PC->IsLocalController())
	{
		if (FireCameraShake)
		{
			PC->ClientStartCameraShake(FireCameraShake, 1);
		}
		if (FireForceFeedback)
		{
			FForceFeedbackParameters FFParams;
			FFParams.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
		}
	}
}

void ARangedWeaponBase::StopSimulatingWeaponFire()
{
	if (bLoopedMuzzleFX )
	{
		if( MuzzlePSC != NULL )
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
		if( MuzzlePSCSecondary != NULL )
		{
			MuzzlePSCSecondary->DeactivateSystem();
			MuzzlePSCSecondary = NULL;
		}
	}

	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}

void ARangedWeaponBase::HandleFiring()
{
	if ( AmmoInClip > 0 && CanFire() )
	{
		if (GetNetMode() != NM_DedicatedServer)
		{
			SimulateWeaponFire();
		}
		
		Fire();
		UseAmmo();

		BurstCounter++;
		
		if ( CurrentState == EWeaponState::Firing && RangedWeaponConfig.TimeBetweenShots > 0.0f )
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &ARangedWeaponBase::HandleReFiring, RangedWeaponConfig.TimeBetweenShots, false);
		}
	}
	else if ( CanReload() )
	{
		StartReload();
	}
	else
	{
		OnBurstFinished();
	}
	
	if ( GetMyPawn() && GetMyPawn()->IsLocallyControlled() )
	{
		if ( GetLocalRole() < ROLE_Authority )
		{
			ServerHandleFiring();
		}

		if ( AmmoInClip <= 0 && CanReload() )
		{
			StartReload();
		}
	}
}

bool ARangedWeaponBase::ServerStartReload_Validate()
{
	return true;
}

void ARangedWeaponBase::ServerStartReload_Implementation()
{
	StartReload();
}

bool ARangedWeaponBase::ServerStopReload_Validate()
{
	return true;
}

void ARangedWeaponBase::ServerStopReload_Implementation()
{
	StopReload();
}

bool ARangedWeaponBase::CanReload() const
{
	const bool bGotAmmo = AmmoInClip < RangedWeaponConfig.ClipSize && (GetMyPawn()->GetInventory()->Contains(RangedWeaponConfig.AmmoClass) || HasInfiniteAmmo());
	const bool bStateOKToReload = CurrentState < EWeaponState::Reloading;
	return bGotAmmo && bStateOKToReload;
}

void ARangedWeaponBase::StartReload(bool bFromReplication)
{
	if ( !bFromReplication && GetLocalRole() < ROLE_Authority )
	{
		ServerStartReload();
	}

	if (bFromReplication || CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		float AnimDuration = PlayWeaponAnimation(ReloadAnim);		
		if (AnimDuration <= 0.0f)
		{
			AnimDuration = RangedWeaponConfig.NoAnimReloadDuration;
		}

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &ARangedWeaponBase::StopReload, AnimDuration, false);
		if ( GetLocalRole() == ROLE_Authority )
		{
			GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &ARangedWeaponBase::ReloadWeapon,
											FMath::Max(0.1f, AnimDuration - 0.1f), false);
		}
	}
}

void ARangedWeaponBase::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
	}
}

void ARangedWeaponBase::ReloadWeapon()
{
	if ( HasInfiniteAmmo() )
	{
		AmmoInClip = RangedWeaponConfig.ClipSize;
		return;
	}

	const int32 ClipDelta = RangedWeaponConfig.ClipSize - AmmoInClip;
	const int32 StoredAmmo = GetMyPawn()->GetInventory()->Use(RangedWeaponConfig.AmmoClass, ClipDelta);
	
	if (StoredAmmo > 0)
	{
		AmmoInClip += StoredAmmo;
	}
}

void ARangedWeaponBase::StartFire()
{
	if (GetLocalRole() < ROLE_Authority )
	{
		ServerStartFire();
	}

	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

void ARangedWeaponBase::StopFire()
{
	if ( GetLocalRole() < ROLE_Authority && GetMyPawn() && GetMyPawn()->IsLocallyControlled() )
	{
		ServerStopFire();
	}

	if ( bWantsToFire )
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

bool ARangedWeaponBase::CanFire() const
{
	return CurrentState < EWeaponState::Reloading && bPendingReload == false;
}

FVector ARangedWeaponBase::GetAdjustedAim() const
{
	AController* const OwnerController = GetMyPawn()->Controller;
	APlayerController* PlayerController = Cast<APlayerController>(OwnerController);
	AAIController* AIController = Cast<AAIController>(OwnerController);
	FVector FinalAim = FVector::ZeroVector;

	if (PlayerController)
	{
		FVector CamLoc;
		FRotator CamRot;
		PlayerController->GetPlayerViewPoint(CamLoc, CamRot);
		FinalAim = CamRot.Vector();
	}
	else if (AIController)
	{
		if(AIController)
		{
			FinalAim = AIController->GetControlRotation().Vector();
		}
	}
	else if (GetInstigator())
	{			
		FinalAim = GetInstigator()->GetBaseAimRotation().Vector();
	}
	
	return FinalAim;
}

FVector ARangedWeaponBase::GetCameraDamageStartLocation(const FVector& AimDir) const
{
	AController* const OwnerController = GetMyPawn()->Controller;
	APlayerController* PlayerController = Cast<APlayerController>(OwnerController);
	AAIController* AIController = Cast<AAIController>(OwnerController);
	FVector OutStartTrace = FVector::ZeroVector;
	
	if (PlayerController)
	{
		// use player's camera
		FRotator UnusedRot;
		PlayerController->GetPlayerViewPoint(OutStartTrace, UnusedRot);

		// Adjust trace so there is nothing blocking the ray between the camera and the pawn, and calculate distance from adjusted start
		OutStartTrace = OutStartTrace + AimDir * (GetMyPawn()->GetActorLocation() - OutStartTrace | AimDir);
	}
	else if (AIController)
	{
		OutStartTrace = GetMuzzleLocation();
	}
	
	return OutStartTrace;
}

FVector ARangedWeaponBase::GetMuzzleLocation() const
{
	USkeletalMeshComponent* UseMesh = GetWeaponMesh();
	return UseMesh->GetSocketLocation(MuzzleAttachPoint);
}
