// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MortalCryPlayerController.generated.h"

DECLARE_DYNAMIC_DELEGATE(FInteractSignature);
DECLARE_DYNAMIC_DELEGATE(FEndInteractSignature);
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

	FTimerHandle InteractTimer;

	UPROPERTY()
	FInteractSignature OnInteract;

	UPROPERTY()
	FEndInteractSignature OnEndInteract;
	
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	APawn* MainPawn;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    APawn* Trace();

	UFUNCTION(BlueprintCallable)
    void Interact();

	UFUNCTION(BlueprintCallable)
    void Interact_Released();
	
protected:
	virtual void SetupInputComponent() override;
	
	UFUNCTION(BlueprintCallable, Reliable, Server)
	void DoUnPossess();

	void Interact_Internal();

	virtual void OnUnPossess() override;
	
public:
	virtual void Tick(float DeltaSeconds) override;
	
};
