// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Informative.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInformative : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class MORTALCRY_API IInformative
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FString GetName() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void GetDescription(FString& OutString) const;
	
};