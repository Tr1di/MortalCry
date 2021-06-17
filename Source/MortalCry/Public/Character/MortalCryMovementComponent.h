// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MortalCryMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MORTALCRY_API UMortalCryMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient, Replicated)
	bool bWantsToWalk;
	
	UPROPERTY(Transient, Replicated)
	bool bWantsToRun;
	
public:
	explicit UMortalCryMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SlowWalkSpeedModifier;

	UPROPERTY(Category="Character Movement: Walking", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float RunSpeedModifier;
	
protected:
	void SetWalk(bool bNewWalking);
	void SetRunning(bool bNewRunning);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetWalking(bool bNewWalking);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetRunning(bool bNewRunning);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	void OnStartWalking();
	void OnStopWalking();
	
	void OnStartRunning();
	void OnStopRunning();
	
	bool IsSlowWalking() const;
	bool IsRunning() const;
	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void Crouch(bool bClientSimulation) override;
	
	virtual float GetMaxSpeed() const override;
	
	virtual bool CanCrouchInCurrentState() const override;
};
