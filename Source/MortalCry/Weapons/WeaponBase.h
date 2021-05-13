// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Weapon.h"
#include "Animation/SkeletalMeshActor.h"
#include "MortalCry/Collectable.h"
#include "MortalCry/Informative.h"

#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class MORTALCRY_API AWeaponBase : public ASkeletalMeshActor, public IWeapon, public ICollectable, public IInformative
{
	GENERATED_BODY()

public:
	explicit AWeaponBase(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FString InteractText;
	
public:
	virtual void Sheath_Implementation() override;
	
	virtual void Interact_Implementation(AActor* InInstigator) override;
	virtual void EndInteract_Implementation() override;

	virtual FString GetDescription_Implementation() const override;
	
	virtual FString GetName_Implementation() const override { return Name.ToString(); }
	
};
