// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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
    void EndAttack();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void AlterAttack();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void EndAlterAttack();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void Action();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void EndAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void AlterAction();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void EndAlterAction();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Draw();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Sheath();
};
