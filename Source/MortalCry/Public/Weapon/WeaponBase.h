// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Weapon.h"
#include "Animation/SkeletalMeshActor.h"
#include "Character/MortalCryCharacter.h"
#include "Inventory/Collectable.h"
#include "Sound/SoundCue.h"
#include "UI/Informative.h"

#include "WeaponBase.generated.h"

USTRUCT(BlueprintType)
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* PawnFP;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UAnimMontage* PawnTP;
};

/**
 * 
 */
UCLASS(Abstract)
class MORTALCRY_API AWeaponBase : public ASkeletalMeshActor, public ICollectable, public IInformative
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* MeshFP;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FName WeaponType;
	
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	AMortalCryCharacter* MyPawn;
	
	void DetachFromPawn();
	void AttachToPawn();
	
public:
	explicit AWeaponBase(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable)
	virtual float PlayWeaponAnimation(const FWeaponAnim& Animation);
	void StopWeaponAnimation(const FWeaponAnim& Animation);
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	virtual void Attack() PURE_VIRTUAL(AWeaponBase::Attack,);
	virtual void StopAttacking() PURE_VIRTUAL(AWeaponBase::StopAttacking,);

	virtual void AlterAction() PURE_VIRTUAL(AWeaponBase::AlterAction,)

	FORCEINLINE FName GetType() const { return WeaponType; }
	
	virtual void OnUnEquip();
	virtual void Interact_Implementation(AActor* InInstigator) override;
	virtual void StopInteracting_Implementation() override;
	virtual FText GetDescription_Implementation() const override;
	virtual FText GetName_Implementation() const override { return Name; }

	AMortalCryCharacter* GetMyPawn() const { return MyPawn; }

	USkeletalMeshComponent* GetWeaponMesh() const;
	
	FORCEINLINE USkeletalMeshComponent* GetMeshFP() const { return MeshFP; }
	FORCEINLINE USkeletalMeshComponent* GetMeshTP() const { return GetSkeletalMeshComponent(); }
};
