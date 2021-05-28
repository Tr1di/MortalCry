// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "Collectable.h"
#include "Informative.h"
#include "Usable.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	
	
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
		FCollectedItem NewCollectedItem;
		
		NewCollectedItem.Item = Item->GetClass();
		NewCollectedItem.Icon = ICollectable::Execute_GetIcon(Item);
		NewCollectedItem.Name = IInformative::Execute_GetName(Item);
		NewCollectedItem.Description = IInformative::Execute_GetDescription(Item);
		NewCollectedItem.Amount = ICollectable::Execute_GetSize(Item);
		
		Collect(NewCollectedItem);
		
		IInteractive::Execute_Interact(Item, GetOwner());
	}
}

void UInventoryComponent::Collect(FCollectedItem Item)
{
	if ( !Items.Contains(Item) )
	{
		Items.Add(Item);
	}
	else
	{
		for (FCollectedItem& I : Items)
		{
			if (I == Item)
			{
				I += Item;
			}
		}
	}
}

int32 UInventoryComponent::Get(TSubclassOf<AActor> ItemClass, int32 Amount)
{
	FCollectedItem SearchedItem;
	SearchedItem.Item = ItemClass;
	SearchedItem.Amount = -Amount;

	FCollectedItem Item = GetItem(ItemClass);
	
	if ( Item == SearchedItem )
	{
		const bool IsEnough = Item.Amount > Amount;
		const int32 Result = IsEnough ? Amount : Item.Amount;
		
		Collect(SearchedItem);

		if ( !IsEnough )
		{
			Items.Remove(Item);
			if (EquippedItem && EquippedItem->GetClass() == Item.Item)
			{
				Equip(0, true);
			}
		}
		
		return Result;
	}
	
	return 0;
}

FCollectedItem UInventoryComponent::GetItem(TSubclassOf<AActor> ItemClass) const
{
	for (auto Item : Items)
	{
		if (Item.Item == ItemClass)
		{
			return Item;
		}
	}
	return FCollectedItem();
}

void UInventoryComponent::Equip(int32 Index, bool IsValid)
{
	if ( GetEquippedItem() )
	{
		UE_LOG(LogTemp, Warning, TEXT("Destroy EquippedItem"))
		EquippedItem->Destroy();
	}
	
	EquippedItem = nullptr;
	
	if ( IsValid && Items.IsValidIndex(Index) )
	{
		const FCollectedItem SelectedItem = Items[Index];
		
		if ( SelectedItem.Item->ImplementsInterface(UUsable::StaticClass()) )
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			AActor* Item = GetWorld()->SpawnActor(SelectedItem.Item, 0, 0, Params);
			Item->SetActorEnableCollision(false);
			EquippedItem = Item;
			//EquippedItem->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		}
	}
}



bool UInventoryComponent::CanCollect(AActor* Item) const
{
	if (!Item || !Item->Implements<UCollectable>()) { return false; }
	if ( Items.Num() == MaxItems ) { return false; }

	const int32 ItemSize = ICollectable::Execute_GetSize(Item);
	int32 FreeSpace = Size;
	
	for ( const auto I : Items)
	{
		FreeSpace -= I.Amount;
	}
	
	return ItemSize <= FreeSpace;
}

