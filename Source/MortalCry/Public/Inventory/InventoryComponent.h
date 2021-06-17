// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"

#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FCollectedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	TSubclassOf<AActor> Item;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Item)
	FString Description;
	
	friend bool operator==(const FCollectedItem& Lhs, const FCollectedItem& RHS)
	{
		return Lhs.Item == RHS.Item;
	}

	friend bool operator!=(const FCollectedItem& Lhs, const FCollectedItem& RHS) { return !(Lhs == RHS); }

	friend uint32 GetTypeHash(const FCollectedItem& CollectedItem)
	{
		return GetTypeHash(CollectedItem.Item)
			+ GetTypeHash(CollectedItem.Icon)
			+ GetTypeHash(CollectedItem.Name)
			+ GetTypeHash(CollectedItem.Description);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MORTALCRY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	int32 Size;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	int32 MaxItems;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	TMap<FCollectedItem, int32> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	AActor* EquippedItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	FName AttachSocketName; 
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UFUNCTION(BlueprintCallable)
	void Collect(AActor* Item);

	UFUNCTION(BlueprintCallable)
	bool Contains(TSubclassOf<AActor> ItemClass);
	
	UFUNCTION(BlueprintCallable)
	int32 Use(TSubclassOf<AActor> ItemClass, int32 Amount = 1);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Equip(int32 Index, bool IsValid);

protected:
	int32 GetUsedSpace() const;
	
	UFUNCTION(Server, Reliable)
	void CollectItem(FCollectedItem Item, int32 Amount = 1);
	
	UFUNCTION()
	bool CanCollect(AActor* Item) const;
	
public:
	virtual void UseEquippedItem();
	
	FORCEINLINE AActor* GetEquippedItem() const { return EquippedItem; }
		
};
