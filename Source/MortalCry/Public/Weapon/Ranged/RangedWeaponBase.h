// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "RangedWeapon.h"
#include "Weapon/WeaponBase.h"
#include "RangedWeaponBase.generated.h"

UENUM(BlueprintType)
namespace ERangedWeaponState
{
	enum Type
	{
		Idle		UMETA(DisplayName = "Idle"		),
		Firing		UMETA(DisplayName = "Firing"	),
		Reloading	UMETA(DisplayName = "Reloading"	),
		Equipping	UMETA(DisplayName = "Equipping"	),
		MAX
	};
}

USTRUCT()
struct FRangedWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** inifite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	bool bInfiniteAmmo;

	/** infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	bool bInfiniteClip;

	/** clip size */
	UPROPERTY(EditAnywhere, Category=Ammo)
	int32 ClipSize;

	/** time between two consecutive shots */
	UPROPERTY(EditAnywhere, Category=WeaponStat)
	float TimeBetweenShots;

	/** failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditAnywhere, Category=WeaponStat)
	float NoAnimReloadDuration;

	UPROPERTY(EditAnywhere, Category=Ammo)
	TSubclassOf<AActor> AmmoClass;
	
	/** defaults */
	FRangedWeaponData()
	{
		bInfiniteAmmo = false;
    	bInfiniteClip = false;
		ClipSize = 20;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
	}
};

/**
 * 
 */
UCLASS()
class MORTALCRY_API ARangedWeaponBase : public AWeaponBase, public IRangedWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Config)
	FRangedWeaponData RangedWeaponConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Ranged, meta = (AllowPrivateAccess = "true"))
	int32 AmmoInClip;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animations, meta = (AllowPrivateAccess = "true"))
	FWeaponAnim ReloadAnim;
	
	bool bWantsToFire;
	bool bPendingReload;

	int32 BurstCounter;
	
	FTimerHandle TimerHandle_HandleFiring;
	FTimerHandle TimerHandle_StopReload;
	FTimerHandle TimerHandle_ReloadWeapon;

protected:
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireLoopSound;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireSound;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UForceFeedbackEffect* FireForceFeedback;

	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FireFinishSound;
	
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<UMatineeCameraShake> FireCameraShake;
	
	UPROPERTY(Transient)
	UAudioComponent* FireAC;
	
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	bool bLoopedFireSound;
	
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	FWeaponAnim FireAnim;
	
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	bool bLoopedMuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	bool bLoopedFireAnim;

	bool bPlayingFireAnim;
	
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;
	
	UPROPERTY(BlueprintReadOnly, Category = Ranged, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ERangedWeaponState::Type> CurrentState;
	
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName MuzzleAttachPoint;
	
public:
	explicit ARangedWeaponBase(const FObjectInitializer& ObjectInitializer);

protected:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartFire();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopFire();

	void DetermineWeaponState();
	void SetWeaponState(ERangedWeaponState::Type NewState);

	virtual void OnBurstStarted();
	virtual void OnBurstFinished();

	void UseAmmo();
	void HandleReFiring();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleFiring();
	virtual void SimulateWeaponFire();
	void StopSimulatingWeaponFire();
	void HandleFiring();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartReload();
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopReload();
	bool CanReload() const;
	virtual void StartReload(bool bFromReplication = false);
	virtual void StopReload();
	virtual void ReloadWeapon();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	virtual void StartFire();
	virtual void StopFire();
	
	virtual void Fire() PURE_VIRTUAL(ARangedWeaponBase::Fire,);

	bool CanFire() const; 

	virtual void Attack() override { StartFire(); }
	virtual void StopAttacking() override { StopFire(); }

	virtual void AlterAction() override { StartReload(); }
	
	virtual int32 GetAmmo_Implementation() const override { return AmmoInClip; }
	virtual int32 GetMagazinesSize_Implementation() const override { return RangedWeaponConfig.ClipSize; }

	FORCEINLINE bool HasInfiniteAmmo() const { return RangedWeaponConfig.bInfiniteAmmo; }
	FORCEINLINE bool HasInfiniteClip() const { return RangedWeaponConfig.bInfiniteClip; }

	FVector GetAdjustedAim() const;
	FVector GetMuzzleLocation() const;
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;
};
