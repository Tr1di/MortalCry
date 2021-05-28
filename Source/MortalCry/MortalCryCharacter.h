// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GenericTeamAgentInterface.h"
#include "Interactive.h"
#include "InventoryComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Teams/Team.h"

#include "MortalCryCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FInteractSiganture, AActor*, Item );

class AWeaponBase;
class UInputComponent;
class USceneComponent;
class USkeletalMeshComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct FInventoryData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MustImplement = "WeaponBase"))
	TArray<TSubclassOf<AActor>> Weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TSubclassOf<AActor>, uint8> Items;
};

USTRUCT(BlueprintType)
struct FHolsters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Holsters;
};

UCLASS(config=Game)
class AMortalCryCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* MeshFP;
	
	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TMap<FName, FHolsters> Holsters;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true", MustImplement = "Weapon"))
	TArray<AActor*> Weapons;
	
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true", MustImplement = "Weapon"))
	AActor* CurrentWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETeam::Type> Team;

public:
	explicit AMortalCryCharacter(const FObjectInitializer& ObjectInitializer);

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
	//////////
	// Health
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	float FullHealth;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Replicated, Category = Health, meta = (AllowPrivateAccess = "true"))
	float Health;

public:
	UFUNCTION(BlueprintPure, Category = Health)
	float GetHealth() const;

	UFUNCTION(BlueprintPure, Category = Health)
	bool IsAlive() const { return Health > 0.f; }
	
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = Health)
	void UpdateHealth(float HealthChange);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	UInventoryComponent* Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	float InventoryOpenDelay;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	bool bIsInventoryOpen;

protected:
	FTimerHandle InventoryTimer;
	
	UFUNCTION(BlueprintCallable, Category = Inventory)
	void OpenInventory();

public:
	AActor* GetEquippedItem() const { return Inventory->GetEquippedItem(); }
	
	UFUNCTION(BlueprintPure, Category = Inventory)
	bool IsInventoryOpen() const { return bIsInventoryOpen; }
	
	//////////
	// Interact
private:
	UPROPERTY(BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess = "true", MustImplement = "Interactive"))
	AActor* ActualInteractiveActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	float InteractLength;

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* InteractTrace(TSubclassOf<UInterface> SearchClass);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void PickUp(AActor* Item);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Drop(AActor* Item);

protected:
	UPROPERTY(BlueprintAssignable)
	FInteractSiganture OnPickUp;

	UPROPERTY(BlueprintAssignable)
	FInteractSiganture OnDrop;

private:
	UFUNCTION()
	void OnPickUpWeapon(AActor* Item);
	
	UFUNCTION()
	void OnPickUpItem(AActor* Item);

	UFUNCTION()
	void OnDropWeapon(AActor* Item);

protected:	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void EndInteract();

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* InInteractiveActor);
	
	UFUNCTION(Server, Reliable)
	void ServerStopInteract();

public:
	UFUNCTION()
	FName GetSocketFor(AActor* Weapon);

protected:
	UFUNCTION(BlueprintCallable)
	void Attack();
	
	UFUNCTION(BlueprintCallable)
	void StopAttacking();
	
	UFUNCTION(BlueprintCallable)
	void AlterAttack();
	
	UFUNCTION(BlueprintCallable)
	void StopAlterAttack();
	
	UFUNCTION(BlueprintCallable)
	void Action();
	
	UFUNCTION(BlueprintCallable)
	void StopAction();
	
	UFUNCTION(BlueprintCallable)
	void AlterAction();
	
	UFUNCTION(BlueprintCallable)
	void StopAlterAction();

	UFUNCTION(BlueprintCallable)
	void NextWeapon();

	UFUNCTION(BlueprintCallable)
	void PreviousWeapon();

	UFUNCTION(BlueprintCallable)
	void SheathActualWeapon();

	UFUNCTION(BlueprintCallable)
	void DropActualWeapon();

	UFUNCTION(BlueprintCallable)
	void BeginUse();

	UFUNCTION(BlueprintCallable)
	void Use();
	
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
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetActualWeapon(AActor* NewWeapon);

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Draw(AActor* Weapon);
	
	UFUNCTION(NetMulticast, Reliable)
	void Sheath(AActor* Weapon, FName SocketName = NAME_None);

public:
	void Crouch();
	void UnCrouch();
	
	UFUNCTION(BlueprintCallable)
	void SwitchCrouch();
	
protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;
	
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
							AActor* DamageCauser) override;
	
	virtual USceneComponent* GetDefaultAttachComponent() const override { return IsLocallyControlled() && IsPlayerControlled() ? GetMeshFP() : GetMesh(); }
	
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(Team); }
	
	/** Returns MeshFP subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMeshFP() const { return MeshFP; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
};

/**
* Hehehe :)
*/
template<typename T>
T UselessFunction(T Input)
{
	return Input;
}