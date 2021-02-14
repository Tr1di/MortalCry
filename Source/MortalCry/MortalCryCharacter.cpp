// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryCharacter.h"

#include "MortalCryProjectile.h"
#include "MotionControllerComponent.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Engine/DemoNetDriver.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/Weapon.h"

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

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	MeshFP = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMeshFP"));
	MeshFP->SetupAttachment(RootComponent);
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
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AMortalCryCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMortalCryCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMortalCryCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMortalCryCharacter::LookUpAtRate);
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
	if ( GetLocalRole() == ROLE_Authority )
	{
		OnPickUp.Broadcast(Item);
	}
}

void AMortalCryCharacter::OnPickUpWeapon_Implementation(AActor* Item)
{
	if ( Weapons.Num() > 3 )
	{
		return;
	}
	
	if ( Item && Item->Implements<UWeaponBase>() )
	{
		if (Item && Item->Implements<UInteractive>() )
		{
			IInteractive::Execute_Interact(Item, this);
		}
		
		Weapons.Add(Item);
		MoveIgnoreActorAdd(Item);
		
		if ( !ActualWeapon )
		{
			SetActualWeapon(Item);
			return;
		}

		Item->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("BackSocket"));
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
	
	//if ( ActorLineTraceSingle(OutHit, Start, End, ECC_Visibility, Params) )
	
	if ( UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, TraceTypeQuery2, true, ActorsToIgnore,
                                            EDrawDebugTrace::None, OutHit, true) )
	{
		if ( OutHit.GetActor()->Implements<UInteractive>() )
		{
			return OutHit.GetActor();
		}
	}

	return nullptr;
}

float AMortalCryCharacter::PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	const float ParentResult = 0; //Super::PlayAnimMontage(AnimMontage, InPlayRate, StartSectionName);
	
	UAnimInstance* AnimInstance = Mesh1P ? Mesh1P->GetAnimInstance() : nullptr; 
	if( AnimMontage && AnimInstance )
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if ( Duration > 0.f )
		{
			if( StartSectionName != NAME_None )
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return ParentResult;
}

void AMortalCryCharacter::OnAttack()
{
	//if ( GetLocalRole() == ROLE_Authority )
	//{
		if ( ActualWeapon )
		{
			IWeaponBase::Execute_Attack(ActualWeapon);
		}
	//}
}

void AMortalCryCharacter::OnEndAttack()
{
	//if ( GetLocalRole() == ROLE_Authority )
	//{
		if ( ActualWeapon )
		{
			IWeaponBase::Execute_EndAttack(ActualWeapon);
		}
	//}
}

void AMortalCryCharacter::OnAlterAttack()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_AlterAttack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAlterAttack()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_EndAlterAttack(ActualWeapon);
	}
}

void AMortalCryCharacter::OnAction()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_Action(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAction()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_EndAction(ActualWeapon);
	}
}

void AMortalCryCharacter::OnAlterAction()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_AlterAction(ActualWeapon);
	}
}

void AMortalCryCharacter::OnEndAlterAction()
{
	if ( ActualWeapon )
	{
		IWeaponBase::Execute_EndAlterAction(ActualWeapon);
	}
}

bool AMortalCryCharacter::ServerInteract_Validate(AActor* InInteractiveActor)
{
	return true;
}

void AMortalCryCharacter::ServerInteract_Implementation(AActor* InInteractiveActor)
{
	if ( InInteractiveActor->Implements<UCollectable>() )
	{
		PickUp(InInteractiveActor);
		return;
	}
	
	ActualInteractiveActor = InInteractiveActor;
}

void AMortalCryCharacter::Interact_Implementation()
{
	if ( AActor* Interactive = InteractTrace() )
	{
		ServerInteract(Interactive);
	}
}

void AMortalCryCharacter::EndInteract_Implementation()
{
	
}

void AMortalCryCharacter::ServerSetActualWeapon_Implementation(AActor* NewWeapon)
{
	MulticastSetActualWeapon(NewWeapon);
}

void AMortalCryCharacter::MulticastSetActualWeapon_Implementation(AActor* NewWeapon)
{
	if ( NewWeapon )
	{
		if (ActualWeapon)
		{
			ActualWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("BackSocket"));
		}
		ActualWeapon = NewWeapon;
		ActualWeapon->AttachToComponent(MeshFP, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
	}
}

bool AMortalCryCharacter::SetActualWeapon(AActor* NewWeapon)
{
	if ( NewWeapon && NewWeapon != ActualWeapon )
	{
		ServerSetActualWeapon(NewWeapon);
		return true;
	}
	return  false;
}

void AMortalCryCharacter::NextWeapon()
{
	int32 Index = Weapons.Find(ActualWeapon) + 1;
	while( !SetActualWeapon(Weapons[Index % Weapons.Num()]) && ++Index < Weapons.Num() );
}

void AMortalCryCharacter::PreviousWeapon()
{
	int32 Index = Weapons.Find(ActualWeapon) + Weapons.Num() - 1;
	while( !SetActualWeapon(Weapons[Index % Weapons.Num()]) && --Index > Weapons.Num() );
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
	return ActualDamage;
}
