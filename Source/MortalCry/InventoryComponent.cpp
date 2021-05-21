// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "Collectable.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	
	
	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UInventoryComponent::Collect(AActor* Item)
{
	if ( CanCollect(Item) )
	{
		const auto ItemClass = Item->GetClass();
		
		if ( !Items.Contains(ItemClass) )
		{
			Items.Add(ItemClass);
		}
		else
		{
			Items[ItemClass] += ICollectable::Execute_GetSize(Item);
		}
		
		IInteractive::Execute_Interact(Item, GetOwner());
	}
}

bool UInventoryComponent::CanCollect(AActor* Item) const
{
	if (!Item || !Item->Implements<UCollectable>())
	{
		return false;
	}

	const int32 ItemSize = ICollectable::Execute_GetSize(Item);
	int32 FreeSpace = Size;

	for ( const auto I : Items)
	{
		FreeSpace -= I.Value;
	}
	
	return ItemSize <= FreeSpace;
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

