// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GenericTeamAgentInterface.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "Teams/Team.h"

#include "MortalCryCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FPickUpSiganture, AActor*, Item );

class AWeaponBase;
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
	AActor* ActualWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TMap<TSubclassOf<AActor>, uint8> Items;

	UPROPERTY(BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess = "true", MustImplement = "Interactive"))
	AActor* ActualInteractiveActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Health, meta = (AllowPrivateAccess = "true"))
	float FullHealth;
	
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Replicated, Category = Health, meta = (AllowPrivateAccess = "true"))
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction, meta = (AllowPrivateAccess = "true"))
	float InteractLength;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Team, meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETeam::Type> Team;

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

	UFUNCTION(BlueprintPure, Category = Health)
	float GetHealth();

	UFUNCTION(BlueprintPure, Category = Health)
	FText GetHealthText();

	UFUNCTION(BlueprintCallable, Category = Health)
	void UpdateHealth(float HealthChange);
	
	virtual float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

protected:
	UFUNCTION(BlueprintCallable)
	void OnAttack();
	
	UFUNCTION(BlueprintCallable)
    void OnEndAttack();
	
	UFUNCTION(BlueprintCallable)
	void OnAlterAttack();
	
	UFUNCTION(BlueprintCallable)
    void OnEndAlterAttack();
	
	UFUNCTION(BlueprintCallable)
	void OnAction();
	
	UFUNCTION(BlueprintCallable)
    void OnEndAction();
	
	UFUNCTION(BlueprintCallable)
	void OnAlterAction();
	
	UFUNCTION(BlueprintCallable)
    void OnEndAlterAction();

	UFUNCTION(BlueprintCallable)
	void NextWeapon();

	UFUNCTION(BlueprintCallable)
	void PreviousWeapon();

	UFUNCTION(BlueprintCallable)
	void OnSheathWeapon();

	UFUNCTION()
	void OnDropItem();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Draw(AActor* Weapon);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Sheath(AActor* Weapon, FName SocketName = NAME_None);
	
	UFUNCTION(NetMulticast, Reliable)
	void OnPickUpWeapon(AActor* Item);
	
	UFUNCTION()
	void OnPickUpItem(AActor* Item);

	UFUNCTION()
	FName GetSocketFor(AActor* Weapon);

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* InInteractiveActor);
	
	UFUNCTION(Server, Reliable)
	void ServerEndInteract();

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
	void SetActualWeapon(AActor* NewWeapon);

public:
	void OnCrouch();
	void OnEndCrouch();
	
	UFUNCTION(BlueprintCallable)
	void OnCrouchSwitch();
	
protected:
	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
							AActor* DamageCauser) override;
	
	virtual USceneComponent* GetDefaultAttachComponent() const override { return IsPlayerControlled() && IsLocallyControlled() ? GetMeshFP() : GetMesh(); }
	
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