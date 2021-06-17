// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/MortalCryCharacter.h"

#include "Inventory/Collectable.h"
#include "Interactive.h"
#include "Character/MortalCryMovementComponent.h"
#include "Player/MortalCryPlayerController.h"
#include "MotionControllerComponent.h"
#include "Inventory/Usable.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Damage.h"
#include "Weapon/WeaponBase.h"

// #include "../Plugins/Online/OnlineSubsystemSteam/Source/Public/OnlineSubsystemSteam.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMortalCryCharacter

AMortalCryCharacter::AMortalCryCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMortalCryMovementComponent>(CharacterMovementComponentName))
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

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	
	// Default offset from the character location for projectiles to spawn
	// GunOffset = FVector(100.0f, 0.0f, 10.0f);

    bReplicates = true;

	InteractLength = 150.f;
	
	InventoryOpenDelay = 0.2f;
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
		for ( AWeaponBase* Weapon : Weapons )
		{
			if (Weapon)
			{
				//IInteractive::Execute_Interact(Weapon, this);

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
	PlayerInputComponent->BindAction("ToggleCrouch", IE_Released, this, &AMortalCryCharacter::ToggleCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMortalCryCharacter::UnCrouch);
	
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMortalCryCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMortalCryCharacter::EndInteract);

	PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AMortalCryCharacter::BeginUse);
	PlayerInputComponent->BindAction("Use", IE_Released, this, &AMortalCryCharacter::Use);
	
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &AMortalCryCharacter::Walk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &AMortalCryCharacter::StopWalking);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AMortalCryCharacter::Run);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AMortalCryCharacter::StopRunning);

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
}

void AMortalCryCharacter::PickUp_Implementation(AActor* Item)
{
	OnPickUp.Broadcast(Item);
}

void AMortalCryCharacter::OnPickUpWeapon(AActor* Item)
{
	if ( !HasAuthority() ) { return; }
	if ( Weapons.Contains(Item) ) { return; }
	if ( AWeaponBase* Weapon = Cast<AWeaponBase>(Item) )
	{
		const FName Holster = GetSocketFor(Weapon);
		if (Holster == NAME_None) { return;	}

		IInteractive::Execute_Interact(Weapon, this);
		Weapons.AddUnique(Weapon);
	
		if ( !CurrentWeapon )
		{
			SetActualWeapon(Weapon);
			return;
		}
	
		Sheath(Weapon, Holster);
	}
}

void AMortalCryCharacter::OnPickUpItem(AActor* Item)
{
	if ( !HasAuthority() ) { return; }
	if ( Item->IsA<AWeaponBase>() ) { return; }

	Inventory->Collect(Item);
}

AActor* AMortalCryCharacter::InteractTrace_Implementation(TSubclassOf<UInterface> SearchClass)
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
			if ( OutHit.GetActor()->GetClass()->ImplementsInterface(SearchClass) )
			{
				return OutHit.GetActor();
			}
		}
	}

	return nullptr;
}

float AMortalCryCharacter::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		return UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);
	}

	return 0.0f;
}

void AMortalCryCharacter::StopAnimMontage(UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOut.GetBlendTime(), AnimMontage);
	}
}

void AMortalCryCharacter::Attack()
{
	if ( CurrentWeapon )
	{
		CurrentWeapon->Attack();
	}
}

void AMortalCryCharacter::StopAttacking()
{
	if ( CurrentWeapon )
	{
		CurrentWeapon->StopAttacking();
	}
}

void AMortalCryCharacter::AlterAttack()
{
	if ( CurrentWeapon )
	{
		//IWeapon::Execute_AlterAttack(CurrentWeapon);
	}
}

void AMortalCryCharacter::StopAlterAttack()
{
	if ( CurrentWeapon )
	{
		//IWeapon::Execute_StopAlterAttack(CurrentWeapon);
	}
}

void AMortalCryCharacter::Action()
{
	if ( CurrentWeapon )
	{
		bTargeting = true;
		OnTargeting();
	}
}

void AMortalCryCharacter::StopAction()
{
	if ( CurrentWeapon )
	{
		bTargeting = false;
		OnStopTargeting();
	}
}

void AMortalCryCharacter::AlterAction()
{
	if ( CurrentWeapon )
	{
		CurrentWeapon->AlterAction();
	}
}

void AMortalCryCharacter::StopAlterAction()
{
	if ( CurrentWeapon )
	{
		//IWeapon::Execute_StopAlterAction(CurrentWeapon);
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
	if ( AActor* Interactive = InteractTrace(UInteractive::StaticClass()) )
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

void AMortalCryCharacter::SetActualWeapon_Implementation(AWeaponBase* NewWeapon)
{
	if ( NewWeapon == CurrentWeapon )
	{
		return;
	}
	
	AWeaponBase* OldActualWeapon = CurrentWeapon;
	
	CurrentWeapon = nullptr;
	Sheath(OldActualWeapon);
	
	CurrentWeapon = NewWeapon;
	Draw(CurrentWeapon);
}

void AMortalCryCharacter::Draw_Implementation(AWeaponBase* Weapon)
{	
	if ( Weapon )
	{
		//IWeapon::Execute_Draw(Weapon);
		
		if ( !IsBotControlled() )
		{
			USkeletalMeshComponent* FP = Weapon->GetMeshFP();
			FP->AttachToComponent(GetMeshFP(),
				FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
				TEXT("GripPointFP"));
		}
	
		USkeletalMeshComponent* TP = Weapon->GetMeshTP();
		TP->AttachToComponent(GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("GripPoint"));
	}
}

void AMortalCryCharacter::Sheath_Implementation(AWeaponBase* Weapon, FName SocketName)
{
	if ( Weapon )
	{
		if (SocketName == NAME_None)
		{
			SocketName = GetSocketFor(Weapon);
		}
	
		if (SocketName == NAME_None)
		{
			return;
		}
		
		USkeletalMeshComponent* FP = Weapon->GetMeshFP();
		USkeletalMeshComponent* TP = Weapon->GetMeshTP();
		
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
		GetController()->SetIgnoreLookInput(false);
		bIsInventoryOpen = false;
		return;
	}

	Inventory->UseEquippedItem();
}

void AMortalCryCharacter::Drop_Implementation(AActor* Item)
{
	OnDrop.Broadcast(Item);
}

void AMortalCryCharacter::OnDropWeapon(AActor* Item)
{
	if (AWeaponBase* Weapon = Cast<AWeaponBase>(Item))
	{
		Weapons.Remove(Weapon);

		if (CurrentWeapon == Weapon)
		{
			SetActualWeapon(nullptr);
			NextWeapon();
		}

		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		IInteractive::Execute_StopInteracting(Item);
	}
}

FName AMortalCryCharacter::GetSocketFor(AWeaponBase* Weapon)
{
	if ( !Weapon )
	{	
		return NAME_None;
	}
	
	const FName WeaponType = Weapon->GetType();

	if ( !Holsters.Contains(WeaponType) )
	{
		return NAME_None;
	}
	
	TArray<FName> AllowedHolsters = Holsters[WeaponType].Holsters;
	
	ForEachAttachedActors([&](AActor* Actor)
	{
		if (Actor->IsA<AWeaponBase>())
		{
			AllowedHolsters.Remove(Actor->GetAttachParentSocketName());
		}
		return true;
	});

	if ( CurrentWeapon && CurrentWeapon->GetType() == WeaponType )
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

void AMortalCryCharacter::Walk()
{
	if ( GetMortalCryMovement() )
	{
		GetMortalCryMovement()->OnStartWalking();
	}
}

void AMortalCryCharacter::StopWalking()
{
	if ( GetMortalCryMovement() )
	{
		GetMortalCryMovement()->OnStopWalking();
	}
}

void AMortalCryCharacter::Run()
{
	if ( GetMortalCryMovement() )
	{
		if (bIsCrouched)
		{
			UnCrouch();
		}
		
		GetMortalCryMovement()->OnStartRunning();
	}
}

void AMortalCryCharacter::StopRunning()
{
	if ( GetMortalCryMovement() )
	{
		GetMortalCryMovement()->OnStopRunning();
	}
}

void AMortalCryCharacter::Crouch()
{
	if ( GetMortalCryMovement()->IsRunning() )
	{
		GetMortalCryMovement()->OnStopRunning();
	}
	
	Super::Crouch();
}

void AMortalCryCharacter::UnCrouch()
{
	Super::UnCrouch();
}

void AMortalCryCharacter::ToggleCrouch()
{
	bIsCrouched ? UnCrouch() : Crouch(); 
}

void AMortalCryCharacter::Destroyed()
{
	Super::Destroyed();
	
	for (AWeaponBase* Weapon : Weapons)
	{
		Drop(Weapon);	
	}
}

void AMortalCryCharacter::OpenInventory()
{
	GetWorldTimerManager().ClearTimer(InventoryTimer);
	GetController()->SetIgnoreLookInput(true);
	bIsInventoryOpen = true;
}

float AMortalCryCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
    AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	Health->Update(-ActualDamage);
	
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

UMortalCryMovementComponent* AMortalCryCharacter::GetMortalCryMovement() const
{
	return Cast<UMortalCryMovementComponent>(GetMovementComponent());
}

USkeletalMeshComponent* AMortalCryCharacter::GetSpecificPawnMesh(bool WantFirstPerson) const
{
	return WantFirstPerson ? GetMeshFP() : GetMesh();
}

bool AMortalCryCharacter::IsFirstPerson() const
{
	return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

USkeletalMeshComponent* AMortalCryCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? MeshFP : GetMesh();
}
