// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "WeaponBase.h"
#include "Animation/SkeletalMeshActor.h"
#include "MortalCry/Collectable.h"
#include "MortalCry/Informative.h"

#include "Weapon.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class MORTALCRY_API AWeapon : public ASkeletalMeshActor, public IWeaponBase, public ICollectable, public IInformative
{
	GENERATED_BODY()

public:
	explicit AWeapon(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	FString InteractText;
	
public:
	virtual void Interact_Implementation(AActor* InInstigator) override;
	virtual void EndInteract_Implementation(AActor* InInstigator) override;

	virtual void GetDescription_Implementation(FString& OutString) const override;
	
	virtual FString GetName_Implementation() const override { return Name.ToString(); }
	
};
