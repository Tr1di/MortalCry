// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryCharacter.h"


#include "Collectable.h"
#include "Interactive.h"
#include "MortalCryPlayerController.h"
#include "MortalCryProjectile.h"
#include "MotionControllerComponent.h"
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
	MeshFP->SetupAttachment(GetMesh());
	MeshFP->SetOnlyOwnerSee(true);
	
	// Default offset from the character location for projectiles to spawn
	// GunOffset = FVector(100.0f, 0.0f, 10.0f);

    bReplicates = true;

	InteractLength = 100.f;
	
	OnPickUp.AddDynamic(this, &AMortalCryCharacter::OnPickUpWeapon);
	OnPickUp.AddDynamic(this, &AMortalCryCharacter::OnPickUpItem);
	
	FullHealth = Health = 1000.f;
}

void AMortalCryCharacter::BeginPlay()
{
	Super::BeginPlay();

	if( IGenericTeamAgentInterface* Agent = Cast<IGenericTeamAgentInterface>(GetController()) )
	{
		UE_LOG(LogTemp, Warning, TEXT("Setting team to %i"), static_cast<uint8>(Team))
		Agent->SetGenericTeamId(static_cast<uint8>(Team));
	}
	
	ForEachAttachedActors([&](AActor* AttachedActor)
	{
		if (AttachedActor && AttachedActor->Implements<UInteractive>())
		{
			IInteractive::Execute_Interact(AttachedActor, this);
			MoveIgnoreActorAdd(AttachedActor);
		}
		return true;
	});
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMortalCryCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMortalCryCharacter::OnAttack);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AMortalCryCharacter::OnEndAttack);

	PlayerInputComponent->BindAction("AlterAttack", IE_Pressed, this, &AMortalCryCharacter::OnAlterAttack);
	PlayerInputComponent->BindAction("AlterAttack", IE_Released, this, &AMortalCryCharacter::OnEndAlterAttack);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AMortalCryCharacter::OnAction);
	PlayerInputComponent->BindAction("Action", IE_Released, this, &AMortalCryCharacter::OnEndAction);

	PlayerInputComponent->BindAction("AlterAction", IE_Pressed, this, &AMortalCryCharacter::OnAlterAction);
	PlayerInputComponent->BindAction("AlterAction", IE_Released, this, &AMortalCryCharacter::OnEndAlterAction);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AMortalCryCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Released, this, &AMortalCryCharacter::PreviousWeapon);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMortalCryCharacter::OnCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMortalCryCharacter::OnEndCrouch);
	PlayerInputComponent->BindAction("CrouchSwitch", IE_Released, this, &AMortalCryCharacter::OnCrouchSwitch);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMortalCryCharacter::Interact);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &AMortalCryCharacter::EndInteract);

	PlayerInputComponent->BindAction("SheathWeapon", IE_Pressed, this, &AMortalCryCharacter::OnSheathWeapon);
	
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

	DOREPLIFETIME(AMortalCryCharacter, ActualWeapon);
	DOREPLIFETIME(AMortalCryCharacter, Weapons);
	
	DOREPLIFETIME(AMortalCryCharacter, FullHealth);
	DOREPLIFETIME(AMortalCryCharacter, Health);
}

void AMortalCryCharacter::PickUp_Implementation(AActor* Item)
{
	if ( HasAuthority() )
	{
		OnPickUp.Broadcast(Item);
	}
}

void AMortalCryCharacter::OnPickUpWeapon_Implementation(AActor* Item)
{	
	if ( Item && Item->Implements<UWeapon>() )
	{
		const FName Holster = GetSocketFor(Item);

		if (Holster != NAME_None)
		{
			IInteractive::Execute_Interact(Item, this);
	
			Weapons.Add(Item);
	
			if ( !ActualWeapon )
			{
				Draw(Item);
				return;
			}
	
			Sheath(Item, Holster);
		}
	}
}

void AMortalCryCharacter::OnPickUpItem(AActor* Item)
{
	
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

void AMortalCryCharacter::OnAttack()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_Attack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAttack()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_EndAttack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnAlterAttack()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_AlterAttack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAlterAttack()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_EndAlterAttack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnAction()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_Action(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAction()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_EndAction(ActualWeapon);
	}
}

void AMortalCryCharacter::OnAlterAction()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_AlterAction(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAlterAction()
{
	if ( ActualWeapon )
	{
		IWeapon::Execute_EndAlterAction(ActualWeapon);
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

void AMortalCryCharacter::Interact_Implementation()
{
	if ( AActor* Interactive = InteractTrace() )
	{
		ServerInteract(Interactive);
	}
}

void AMortalCryCharacter::ServerEndInteract_Implementation()
{
	if ( ActualInteractiveActor )
	{
		IInteractive::Execute_EndInteract(ActualInteractiveActor, this);
		ActualInteractiveActor = nullptr;
	}
}

void AMortalCryCharacter::EndInteract_Implementation()
{
	ServerEndInteract();
}

void AMortalCryCharacter::SetActualWeapon(AActor* NewWeapon)
{	
	ActualWeapon = NewWeapon;
}

void AMortalCryCharacter::NextWeapon()
{
	if (Weapons.Num() != 0)
	{
		const int32 Index = Weapons.Find(ActualWeapon) + 1;
		Draw(Weapons[Index % Weapons.Num()]);
	}
}

void AMortalCryCharacter::PreviousWeapon()
{
	if (Weapons.Num() != 0)
	{
		const int32 Index = Weapons.Find(ActualWeapon) + Weapons.Num() - 1;
		Draw(Weapons[Index % Weapons.Num()]);
	}
}

void AMortalCryCharacter::OnSheathWeapon()
{
	Sheath(ActualWeapon);
}

void AMortalCryCharacter::Draw_Implementation(AActor* Weapon)
{
	if ( Weapon && Weapon->Implements<UWeapon>() )
	{
		Sheath(ActualWeapon);

		SetActualWeapon(Weapon);
		
		IWeapon::Execute_Draw(Weapon);
		Weapon->AttachToComponent(IsPlayerControlled() && IsLocallyControlled() ? MeshFP : GetMesh(),
			FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true),
			TEXT("GripPoint"));
	}
}

void AMortalCryCharacter::Sheath_Implementation(AActor* Weapon, FName SocketName)
{
	if ( Weapon && Weapon->Implements<UWeapon>() )
	{
		if ( Weapon == ActualWeapon )
		{
			SetActualWeapon(nullptr);
		}

		if (SocketName == NAME_None)
		{
			SocketName = GetSocketFor(Weapon);
		}

		if (SocketName == NAME_None)
		{
			return;
		}
		
		IWeapon::Execute_Sheath(Weapon);
		Weapon->AttachToComponent(GetMesh(),FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), SocketName);
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

	if ( ActualWeapon && IWeapon::Execute_GetType(ActualWeapon) == WeaponType )
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

void AMortalCryCharacter::OnCrouch()
{
	Crouch();
}

void AMortalCryCharacter::OnEndCrouch()
{
	UnCrouch();
}

void AMortalCryCharacter::OnCrouchSwitch()
{
	if ( !bIsCrouched )
	{
		OnCrouch();
	}
	else
	{
		OnEndCrouch();
	}
}

float AMortalCryCharacter::GetHealth()
{
	return Health / FullHealth;
}

FText AMortalCryCharacter::GetHealthText()
{
	const int32 HP = FMath::RoundHalfFromZero(GetHealth() * 100.f);
	const FString HPString = FString::FromInt(HP) + FString(TEXT("%"));
	return FText::FromString(HPString);
}

void AMortalCryCharacter::UpdateHealth(float HealthChange)
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
