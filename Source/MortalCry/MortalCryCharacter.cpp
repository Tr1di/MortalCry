// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryCharacter.h"


#include "Collectable.h"
#include "Interactive.h"
#include "MortalCryPlayerController.h"
#include "MortalCryProjectile.h"
#include "MotionControllerComponent.h"
#include "Usable.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "Weapons/Weapon.h"

// #include "../Plugins/Online/OnlineSubsystemSteam/Source/Public/OnlineSubsystemSteam.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMortalCryCharacter

AMortalCryCharacter::AMortalCryCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	SetCanBeDamaged(true);
	
	GetMesh()->SetOwnerNoSee(true);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMeshFP"));
	MeshFP->SetupAttachment(GetFirstPersonCameraComponent());
	MeshFP->SetOnlyOwnerSee(true);

	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	
	// Default offset from the character location for projectiles to spawn
	// GunOffset = FVector(100.0f, 0.0f, 10.0f);

    bReplicates = true;

	InteractLength = 150.f;
	
	FullHealth = Health = 1000.f;
}

void AMortalCryCharacter::BeginPlay()
{
	Super::BeginPlay();

	OnPickUp.AddDynamic(this, &AMortalCryCharacter::OnPickUpWeapon);
	OnPickUp.AddDynamic(this, &AMortalCryCharacter::OnPickUpItem);
	
	OnDrop.AddDynamic(this, &AMortalCryCharacter::OnDropWeapon);
	
	if( IGenericTeamAgentInterface* Agent = Cast<IGenericTeamAgentInterface>(GetController()) )
	{
		UE_LOG(LogTemp, Warning, TEXT("Setting team to %i"), static_cast<uint8>(Team))
		Agent->SetGenericTeamId(static_cast<uint8>(Team));
	}

	if ( !HasAuthority() )
	{
		for ( AActor* Weapon : Weapons )
		{
			if (Weapon)
			{
				IInteractive::Execute_Interact(Weapon, this);

				if ( Weapon == CurrentWeapon )
				{
					Draw(CurrentWeapon);
					continue;
				}
				
				Sheath(Weapon, Weapon->GetAttachParentSocketName());
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMortalCryCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMortalCryCharacter::Attack);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AMortalCryCharacter::StopAttacking);

	PlayerInputComponent->BindAction("AlterAttack", IE_Pressed, this, &AMortalCryCharacter::AlterAttack);
	PlayerInputComponent->BindAction("AlterAttack", IE_Released, this, &AMortalCryCharacter::StopAlterAttack);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AMortalCryCharacter::Action);
	PlayerInputComponent->BindAction("Action", IE_Released, this, &AMortalCryCharacter::StopAction);

	PlayerInputComponent->BindAction("AlterAction", IE_Pressed, this, &AMortalCryCharacter::AlterAction);
	PlayerInputComponent->BindAction("AlterAction", IE_Released, this, &AMortalCryCharacter::StopAlterAction);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AMortalCryCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Released, this, &AMortalCryCharacter::PreviousWeapon);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMortalCryCharacter::Crouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMortalCryCharacter::UnCrouch);
	PlayerInputComponent->BindAction("CrouchSwitch", IE_Released, this, &AMortalCryCharacter::SwitchCrouch);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMortalCryCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMortalCryCharacter::EndInteract);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMortalCryCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMortalCryCharacter::EndInteract);

	PlayerInputComponent->BindAction("DropItem", IE_Released, this, &AMortalCryCharacter::DropActualWeapon);

	PlayerInputComponent->BindAction("SheathWeapon", IE_Released, this, &AMortalCryCharacter::SheathActualWeapon);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMortalCryCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMortalCryCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMortalCryCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMortalCryCharacter::LookUpAtRate);

	if (AMortalCryPlayerController* MCController = Cast<AMortalCryPlayerController>(GetController()))
	{
		MCController->OnTrace.BindDynamic(this, &AMortalCryCharacter::InteractTrace);
	}
}

void AMortalCryCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMortalCryCharacter, CurrentWeapon);
	DOREPLIFETIME(AMortalCryCharacter, Weapons);
	
	DOREPLIFETIME(AMortalCryCharacter, Health);
}

void AMortalCryCharacter::PickUp_Implementation(AActor* Item)
{
	OnPickUp.Broadcast(Item);
}

void AMortalCryCharacter::OnPickUpWeapon(AActor* Item)
{
	if ( !HasAuthority() ) { return; }
	if ( Weapons.Contains(Item) ) { return; }
	if ( !Item || !Item->Implements<UWeapon>() ) { return; }
	
	const FName Holster = GetSocketFor(Item);
	if (Holster == NAME_None) { return;	}

	Item->SetReplicateMovement(false);
	IInteractive::Execute_Interact(Item, this);
	Weapons.AddUnique(Item);
	
	if ( !CurrentWeapon )
	{
		SetActualWeapon(Item);
		return;
	}
	
	Sheath(Item, Holster);
}

void AMortalCryCharacter::OnPickUpItem(AActor* Item)
{
	if ( !Item ) { return; }
	if ( !Item->Implements<UCollectable>() ) { return; }

	Inventory->Collect(Item);
}

AActor* AMortalCryCharacter::InteractTrace_Implementation()
{
	FHitResult OutHit;
	
	const FVector Start = GetPawnViewLocation();
	const FVector Forward = GetControlRotation().Vector();
	const FVector End = Start + Forward * InteractLength;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	GetAttachedActors(ActorsToIgnore, false);

	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	
	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params))
	{
		if (OutHit.bBlockingHit && OutHit.GetActor())
		{
			if ( OutHit.GetActor()->Implements<UInteractive>() )
			{
				return OutHit.GetActor();
			}
		}
	}

	return nullptr;
}

float AMortalCryCharacter::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	return Super::PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
}

void AMortalCryCharacter::Attack()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_Attack(CurrentWeapon);
	}
}

void AMortalCryCharacter::StopAttacking()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_StopAttacking(CurrentWeapon);
	}
}

void AMortalCryCharacter::AlterAttack()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_AlterAttack(CurrentWeapon);
	}
}

void AMortalCryCharacter::StopAlterAttack()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_StopAlterAttack(CurrentWeapon);
	}
}

void AMortalCryCharacter::Action()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_Action(CurrentWeapon);
	}
}

void AMortalCryCharacter::StopAction()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_StopAction(CurrentWeapon);
	}
}

void AMortalCryCharacter::AlterAction()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_AlterAction(CurrentWeapon);
	}
}

void AMortalCryCharacter::StopAlterAction()
{
	if ( CurrentWeapon )
	{
		IWeapon::Execute_StopAlterAction(CurrentWeapon);
	}
}

bool AMortalCryCharacter::ServerInteract_Validate(AActor* InInteractiveActor)
{
	if ( InInteractiveActor )
	{
		return GetWorld()->ContainsActor(InInteractiveActor);
	}
	return true;
}

void AMortalCryCharacter::ServerInteract_Implementation(AActor* InInteractiveActor)
{
	if ( HasAuthority() )
	{
		if ( InInteractiveActor )
		{
			if ( InInteractiveActor->Implements<UCollectable>() )
			{
				PickUp(InInteractiveActor);
				return;
			}

			ActualInteractiveActor = InInteractiveActor;
			IInteractive::Execute_Interact(ActualInteractiveActor, this);
		}
	}
}

void AMortalCryCharacter::Interact_Implementation()
{
	if ( AActor* Interactive = InteractTrace() )
	{
		ServerInteract(Interactive);
	}
}

void AMortalCryCharacter::ServerStopInteract_Implementation()
{
	if ( ActualInteractiveActor )
	{
		IInteractive::Execute_StopInteracting(ActualInteractiveActor);
		ActualInteractiveActor = nullptr;
	}
}

void AMortalCryCharacter::EndInteract_Implementation()
{
	ServerStopInteract();
}

void AMortalCryCharacter::NextWeapon()
{
	if (Weapons.Num() != 0)
	{
		const int32 Index = Weapons.Find(CurrentWeapon) + 1;
		SetActualWeapon(Weapons[Index % Weapons.Num()]);
	}
}

void AMortalCryCharacter::PreviousWeapon()
{
	if (Weapons.Num() != 0)
	{
		const int32 Index = Weapons.Find(CurrentWeapon) + Weapons.Num() - 1;
		SetActualWeapon(Weapons[Index % Weapons.Num()]);
	}
}

void AMortalCryCharacter::SetActualWeapon_Implementation(AActor* NewWeapon)
{
	if ( NewWeapon == CurrentWeapon )
	{
		return;
	}
	
	AActor* OldActualWeapon = CurrentWeapon;
	
	CurrentWeapon = nullptr;
	Sheath(OldActualWeapon);
	
	CurrentWeapon = NewWeapon;
	Draw(CurrentWeapon);
}

void AMortalCryCharacter::Draw_Implementation(AActor* Weapon)
{
	if ( Weapon && Weapon->Implements<UWeapon>() )
	{
		IWeapon::Execute_Draw(Weapon);
		
		USkeletalMeshComponent* FP = IWeapon::Execute_GetPlayerSpecifiedMesh(Weapon, true);
		USkeletalMeshComponent* TP = IWeapon::Execute_GetPlayerSpecifiedMesh(Weapon, false);
		
		FP->AttachToComponent(GetMeshFP(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("GripPointFP"));
		TP->AttachToComponent(GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("GripPoint"));
	}
}

void AMortalCryCharacter::Sheath_Implementation(AActor* Weapon, FName SocketName)
{
	if ( Weapon && Weapon->Implements<UWeapon>() )
	{
		if (SocketName == NAME_None)
		{
			SocketName = GetSocketFor(Weapon);
		}

		if (SocketName == NAME_None)
		{
			return;
		}

		IWeapon::Execute_Sheath(Weapon);
		
		USkeletalMeshComponent* FP = IWeapon::Execute_GetPlayerSpecifiedMesh(Weapon, true);
		USkeletalMeshComponent* TP = IWeapon::Execute_GetPlayerSpecifiedMesh(Weapon, false);
		
		FP->AttachToComponent(GetMeshFP(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			SocketName);
		TP->AttachToComponent(GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			SocketName);
	}
}

void AMortalCryCharacter::SheathActualWeapon()
{
	SetActualWeapon(nullptr);
}

void AMortalCryCharacter::DropActualWeapon()
{
	Drop(CurrentWeapon);
}

void AMortalCryCharacter::BeginUse()
{
	GetWorldTimerManager().SetTimer(InventoryTimer, this, &AMortalCryCharacter::OpenInventory, InventoryOpenDelay);
}

void AMortalCryCharacter::Use()
{
	GetWorldTimerManager().ClearTimer(InventoryTimer);
	
	if ( IsInventoryOpen() )
	{
		bIsInventoryOpen = false;
		return;
	}

	AActor* EquippedItem = GetEquippedItem();
	
	if (EquippedItem && EquippedItem->Implements<UUsable>())
	{
		IUsable::Execute_Use(EquippedItem, this);
	}
}

void AMortalCryCharacter::Drop_Implementation(AActor* Item)
{
	OnDrop.Broadcast(Item);
	if ( Item && Item->Implements<UWeapon>() )
	{
		IInteractive::Execute_StopInteracting(Item);
	}
}

void AMortalCryCharacter::OnDropWeapon(AActor* Item)
{
	if (Item && Item->Implements<UWeapon>())
	{
		Weapons.Remove(Item);

		if (CurrentWeapon == Item)
		{
			SetActualWeapon(nullptr);
			NextWeapon();
		}

		Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

FName AMortalCryCharacter::GetSocketFor(AActor* Weapon)
{
	if ( !Weapon || !Weapon->Implements<UWeapon>())
	{	
		return NAME_None;
	}
	
	const FName WeaponType = IWeapon::Execute_GetType(Weapon);

	if ( !Holsters.Contains(WeaponType) )
	{
		return NAME_None;
	}
	
	TArray<FName> AllowedHolsters = Holsters[WeaponType].Holsters;
	
	ForEachAttachedActors([&](AActor* Actor)
	{
		if (Actor->Implements<UWeapon>())
		{
			AllowedHolsters.Remove(Actor->GetAttachParentSocketName());
		}
		return true;
	});

	if ( CurrentWeapon && IWeapon::Execute_GetType(CurrentWeapon) == WeaponType )
	{
		const int32 Index = AllowedHolsters.Num() - 1;

		if (AllowedHolsters.IsValidIndex(Index))
		{
			AllowedHolsters.RemoveAt(Index);
		}
	}

	return AllowedHolsters.IsValidIndex(0) ? AllowedHolsters[0] : NAME_None;
}

void AMortalCryCharacter::MoveForward(float Value)
{
	if ( Value != 0.f )
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMortalCryCharacter::MoveRight(float Value)
{
	if ( Value != 0.f )
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMortalCryCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMortalCryCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMortalCryCharacter::Crouch()
{
	Super::Crouch();
}

void AMortalCryCharacter::UnCrouch()
{
	Super::UnCrouch();
}

void AMortalCryCharacter::SwitchCrouch()
{
	bIsCrouched ? Super::UnCrouch() : Super::Crouch(); 
}

float AMortalCryCharacter::GetHealth() const
{
	return Health / FullHealth;
}

void AMortalCryCharacter::OpenInventory()
{
	GetWorldTimerManager().ClearTimer(InventoryTimer);
	bIsInventoryOpen = true;
}

void AMortalCryCharacter::UpdateHealth_Implementation(float HealthChange)
{
	Health += HealthChange;
	Health = FMath::Clamp(Health, 0.f, FullHealth);
}

float AMortalCryCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
    AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	UpdateHealth(-ActualDamage);
	
	if (FPointDamageEvent* const PointDamage = (FPointDamageEvent*)&DamageEvent)
	{
		const FHitResult Hit = PointDamage->HitInfo;
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator, ActualDamage, Hit.TraceStart, Hit.ImpactPoint);
	}
	
	return ActualDamage;
}

void AMortalCryCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	if ( HasAuthority() )
	{
		Team = static_cast<ETeam::Type>(TeamID.GetId());
	}
}
