// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


#include "Possessive.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"

#include "MortalCryCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FPickUpSiganture, AActor*, Item );

class AWeapon;
class UInputComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct FInventory : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MustImplement = "WeaponBase"))
	TArray<TSubclassOf<AActor>> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AActor>, uint8> Items;
};

UCLASS(config=Game)
class AMortalCryCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* MeshFP;
	
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadWrite, Category = Weapon, meta = (AllowPrivateAccess="true", MustImplement = "WeaponBase"))
	AActor* ActualWeapon;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess="true", MustImplement = "WeaponBase"))
	TArray<AActor*> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess="true"))
	TMap<TSubclassOf<AActor>, uint8> Items;

	UPROPERTY(BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess="true", MustImplement = "Interactive"))
	AActor* ActualInteractiveActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess="true"))
	float InteractLength;

protected:
	UPROPERTY(BlueprintAssignable)
	FPickUpSiganture OnPickUp;

public:
	AMortalCryCharacter(const FObjectInitializer& ObjectInitializer);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void PickUp(AActor* Item);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* InteractTrace();
	
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

protected:
	UFUNCTION()
	void OnAttack();
	
	UFUNCTION()
    void OnEndAttack();
	
	UFUNCTION()
	void OnAlterAttack();
	
	UFUNCTION()
    void OnEndAlterAttack();
	
	UFUNCTION()
	void OnAction();
	
	UFUNCTION()
    void OnEndAction();
	
	UFUNCTION()
	void OnAlterAction();
	
	UFUNCTION()
    void OnEndAlterAction();

	UFUNCTION()
	void NextWeapon();

	UFUNCTION()
	void PreviousWeapon();
	
	UFUNCTION(NetMulticast, Reliable)
	void OnPickUpWeapon(AActor* Item);
	
	UFUNCTION()
	void OnPickUpItem(AActor* Item);

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* InInteractiveActor);

protected:	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndInteract();
	
	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

public:
	UFUNCTION(BlueprintCallable)
	bool SetActualWeapon(AActor* NewWeapon);

private:
	UFUNCTION(Server, Reliable)
	void ServerSetActualWeapon(AActor* NewWeapon);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetActualWeapon(AActor* NewWeapon);

public:
	void OnCrouch();
	void OnEndCrouch();
	void OnCrouchSwitch();
	
protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
};
