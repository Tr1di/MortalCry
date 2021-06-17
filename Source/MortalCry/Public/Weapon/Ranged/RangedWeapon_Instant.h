// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Ranged/RangedWeaponBase.h"
#include "RangedWeapon_Instant.generated.h"

class AImpactEffect;
USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadMax;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float HitDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float AllowedViewDotHitDir;

	/** defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 50.f;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

/**
 * 
 */
UCLASS()
class MORTALCRY_API ARangedWeapon_Instant : public ARangedWeaponBase
{
	GENERATED_BODY()

	float GetCurrentSpread() const;

public:
	explicit ARangedWeapon_Instant(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly, Category=Config)
	FInstantWeaponData InstantConfig;

	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<AImpactEffect> ImpactTemplate;
	
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName TrailTargetParam;
	
	UPROPERTY(Transient, ReplicatedUsing=OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	float CurrentFiringSpread;

	FHitResult WeaponTrace(const FVector& StartTrace, const FVector& EndTrace);

	/** server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	bool ShouldDealDamage(AActor* TestActor) const;

	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);
	
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);
	virtual void Fire() override;

	/** called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint);
	
	UFUNCTION()
	void OnRep_HitNotify();
	
	virtual void OnBurstFinished() override;
};
