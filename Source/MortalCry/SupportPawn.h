// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SupportPawn.generated.h"

DECLARE_DELEGATE_OneParam(FPossessSignature, APawn*)

UCLASS()
class MORTALCRY_API ASupportPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASupportPawn();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pawn")
	float BaseTurnRate;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Pawn")
	float BaseLookUpRate;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    APawn* Trace();
	
protected:
	/** DefaultPawn movement component */
	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPawnMovementComponent* MovementComponent;

	FPossessSignature PossessSignature;
	
	UFUNCTION(BlueprintCallable, Category="Pawn")
    virtual void MoveForward(float Val);

	UFUNCTION(BlueprintCallable, Category="Pawn")
    virtual void MoveRight(float Val);

	UFUNCTION(BlueprintCallable, Category="Pawn")
    virtual void MoveUp_World(float Val);

	UFUNCTION(BlueprintCallable, Category="Pawn")
    virtual void TurnAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Category="Pawn")
    virtual void LookUpAtRate(float Rate);

	void DoPossess();
	
	UFUNCTION(Reliable, Server)
    void ServerDoPossess(APawn* InPawn);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; }

};
