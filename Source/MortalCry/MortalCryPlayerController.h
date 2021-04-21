// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MortalCryPlayerController.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal(AActor*, FTraceSignature);

/**
 * 
 */
UCLASS()
class MORTALCRY_API AMortalCryPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	explicit AMortalCryPlayerController(const FObjectInitializer& ObjectInitializer);
	
	FTraceSignature OnTrace;
	
	UFUNCTION(BlueprintCallable)
    AActor* Trace();
	
protected:
	virtual void SetupInputComponent() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	
};
