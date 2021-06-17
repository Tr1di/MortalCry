// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"

#include "Inventory/Collectable.h"
#include "Inventory/Usable.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "UI/Informative.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	Size = 100;
	MaxItems = 10;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
	DOREPLIFETIME(UInventoryComponent, EquippedItem);
}

void UInventoryComponent::Collect(AActor* Item)
{
	if ( CanCollect(Item) )
	{
		FCollectedItem NewCollectedItem;

		NewCollectedItem.Item = Item->GetClass();
		NewCollectedItem.Icon = ICollectable::Execute_GetIcon(Item);
		NewCollectedItem.Name = IInformative::Execute_GetName(Item).ToString();
		NewCollectedItem.Description = IInformative::Execute_GetDescription(Item).ToString();
		
		CollectItem(NewCollectedItem, ICollectable::Execute_GetSize(Item));
		
		IInteractive::Execute_Interact(Item, GetOwner());
	}
}

void UInventoryComponent::CollectItem_Implementation(FCollectedItem Item, int32 Amount)
{
	if ( !Items.Contains(Item) )
	{
		Items.Add(Item);
	}
	
	Items[Item] += Amount;

	if (Items[Item] < 1)
	{
		Items.Remove(Item);
		
		if (EquippedItem && EquippedItem->GetClass() == Item.Item)
		{
			Equip(0, true);
		}
	}
}

bool UInventoryComponent::Contains(TSubclassOf<AActor> ItemClass)
{
	FCollectedItem SearchedItem;
	SearchedItem.Item = ItemClass;

	return Items.Contains(SearchedItem);
}

int32 UInventoryComponent::Use(TSubclassOf<AActor> ItemClass, int32 Amount)
{
	if (!Contains(ItemClass))
	{
		return 0;
	}
	
	FCollectedItem SearchedItem;
	SearchedItem.Item = ItemClass;
	
	int32 CollectedAmount = Items[SearchedItem];
	const bool IsEnough =  CollectedAmount > Amount;
	const int32 Result = IsEnough ? Amount : CollectedAmount;
	
	CollectItem(SearchedItem, -Result);
	
	return Result;
}

void UInventoryComponent::Equip_Implementation(int32 Index, const bool IsValid)
{
	if (!IsValid)
	{
		return;
	}
	
	if ( GetEquippedItem() )
	{
		EquippedItem->Destroy();
	}
	
	EquippedItem = nullptr;

	TArray<FCollectedItem> CollectedItems;
	Items.GetKeys(CollectedItems);
	
	if ( CollectedItems.IsValidIndex(Index) )
	{
		const FCollectedItem SelectedItem = CollectedItems[Index];
		
		if (SelectedItem.Item->ImplementsInterface(UUsable::StaticClass()))
		{
			FActorSpawnParameters Params;
			Params.Owner = GetOwner();
			Params.bNoFail = true;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			
			const FVector SpawnLocation = GetOwner()->GetActorLocation();
			const FRotator SpawnRotation = GetOwner()->GetActorRotation();
			
			AActor* Item = GetWorld()->SpawnActor(SelectedItem.Item, &SpawnLocation, &SpawnRotation, Params);
			Item->SetActorEnableCollision(false);
			Item->DisableComponentsSimulatePhysics();
			EquippedItem = Item;
			EquippedItem->AttachToActor(GetOwner(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachSocketName);
		}
	}
}

int32 UInventoryComponent::GetUsedSpace() const
{
	int32 Result = 0;
	TArray<int32> ItemsSpace;
	Items.GenerateValueArray(ItemsSpace);
	for (int32 ISize : ItemsSpace)
	{
		Result += ISize;
	}
	return Result;
}

bool UInventoryComponent::CanCollect(AActor* Item) const
{
	if ( !Item || !Item->Implements<UCollectable>() ) { return false; }
	if ( Items.Num() >= MaxItems ) { return false; }

	const int32 ItemSize = ICollectable::Execute_GetSize(Item);
	return GetUsedSpace() + ItemSize <= Size;
}

void UInventoryComponent::UseEquippedItem()
{
	if (!EquippedItem)
	{
		return;
	}

	IUsable::Execute_Use(EquippedItem, GetOwner());
	Use(EquippedItem->GetClass());
}

