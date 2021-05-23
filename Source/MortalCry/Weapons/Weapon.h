// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <ThirdParty/CryptoPP/5.6.5/include/argnames.h>

#include "UObject/Interface.h"
#include "Weapon.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeapon : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MORTALCRY_API IWeapon
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Attack();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void StopAttacking();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void AlterAttack();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void StopAlterAttack();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Action();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void StopAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void AlterAction();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void StopAlterAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Draw();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Sheath();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetType() const;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	USkeletalMeshComponent* GetPlayerSpecifiedMesh(bool bSpecified = false) const;
};
