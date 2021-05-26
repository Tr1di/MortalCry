// Fill out your copyright notice in the Description page of Project Settings.


#include "SupportPawn.h"


#include "MortalCryPlayerController.h"
#include "Possessive.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASupportPawn::ASupportPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	//SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	NetPriority = 3.0f;

	BaseEyeHeight = 0.0f;
	bCollideWhenPlacing = false;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, UFloatingPawnMovement>("MovementComponent");
	MovementComponent->UpdatedComponent = RootComponent;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

// Called when the game starts or when spawned
void ASupportPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void ASupportPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ASupportPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASupportPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ASupportPawn::MoveUp_World);
	PlayerInputComponent->BindAxis("Turn", this, &ASupportPawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASupportPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ASupportPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASupportPawn::LookUpAtRate);

	
	if ( GetController() && GetController()->InputComponent)
	{
		GetController()->InputComponent->BindAction("Interact", IE_DoubleClick, this, &ASupportPawn::DoPossess);
		GetController()->InputComponent->BindAction("PossessMain", IE_DoubleClick, this, &ASupportPawn::DoUnPossess);
	}

	// if (AMortalCryPlayerController* MCController = GetController<AMortalCryPlayerController>())
	// {
	// 	MCController->OnTrace.BindDynamic(this, &ASupportPawn::Trace);
	// }
}

void ASupportPawn::UnPossessed()
{
	SetOlController(GetController());
	Super::UnPossessed();
}

// Called every frame
void ASupportPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

AActor* ASupportPawn::Trace_Implementation()
{	
	FHitResult OutHit;
	const FVector Start = GetPawnViewLocation();
	const FVector ForwardVector = GetControlRotation().Vector();
	const FVector End = Start + ForwardVector * 1000.f;

	TArray<AActor*> ActorsToIgnore({ this });
	GetAttachedActors(ActorsToIgnore, false);
	
	if ( UKismetSystemLibrary::LineTraceSingle(GetWorld(), Start, End, TraceTypeQuery5, true, ActorsToIgnore,
                                                EDrawDebugTrace::None, OutHit, true) )
	{
		if ( APawn* HPawn = Cast<APawn>(OutHit.Actor) )
		{
			if ( !HPawn->IsPlayerControlled() && HPawn->Implements<UPossessive>() && IPossessive::Execute_IsPossessive(HPawn) )
			{
				return HPawn;
			}
		}
	}
	return nullptr;
}

void ASupportPawn::DoPossess()
{	
	if ( APawn* P = Cast<APawn>(Trace()) )
	{
		ServerDoPossess(P);
	}
}

void ASupportPawn::ServerDoPossess_Implementation(APawn* InPawn)
{
	if ( GetLocalRole() == ROLE_Authority )
	{
		GetController()->Possess(InPawn);
		AttachToActor(InPawn, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void ASupportPawn::DoUnPossess()
{
	ServerDoUnPossess();
}

void ASupportPawn::ServerDoUnPossess_Implementation()
{
	if ( GetController() ) return;
	
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if ( OldController.IsValid() )
	{
		OldController->Possess(this);
	}
}

void ASupportPawn::SetOlController_Implementation(AController* InController)
{
	OldController = InController;
}

void ASupportPawn::MoveForward(float Val)
{
	if ( Controller && Val != 0.f )
	{
		FRotator const ControlSpaceRot = Controller->GetControlRotation();

		// transform to world space and add it
		AddMovementInput( FRotationMatrix(ControlSpaceRot).GetScaledAxis( EAxis::X ), Val );
	}
}

void ASupportPawn::MoveRight(float Val)
{
	if ( Controller && Val != 0.f )
	{
		FRotator const ControlSpaceRot = Controller->GetControlRotation();

		// transform to world space and add it
		AddMovementInput( FRotationMatrix(ControlSpaceRot).GetScaledAxis( EAxis::Y ), Val );
	}
}

void ASupportPawn::MoveUp_World(float Val)
{
	if (Val != 0.f)
	{
		AddMovementInput(FVector::UpVector, Val);
	}
}

void ASupportPawn::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}

void ASupportPawn::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}