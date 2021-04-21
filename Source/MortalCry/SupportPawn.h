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
    AActor* Trace();
	
private:
	TWeakObjectPtr<AController> OldController;

	UFUNCTION(Server, Reliable)
	void SetOlController(AController* InController);
	
protected:
	/** DefaultPawn movement component */
	UPROPERTY(Category = Pawn, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPawnMovementComponent* MovementComponent;

	FPossessSignature Possess;
	
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
	
	UFUNCTION(Server, Reliable)
    void ServerDoPossess(APawn* InPawn);

	void DoUnPossess();

	UFUNCTION(Server, Reliable)
    void ServerDoUnPossess();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void UnPossessed() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual UPawnMovementComponent* GetMovementComponent() const override { return MovementComponent; }

};
