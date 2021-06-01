// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FCollectedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> Item;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;
	
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

	friend FCollectedItem operator+(FCollectedItem& Lhs, FCollectedItem& RHS)
	{
		Lhs.Amount += RHS.Amount;
		return Lhs;
	}

	friend FCollectedItem operator+=(FCollectedItem& Lhs, FCollectedItem& RHS)
	{
		return Lhs + RHS;
	}

	friend FCollectedItem operator+(FCollectedItem& Lhs, const int32 RHS)
	{
		Lhs.Amount += RHS;
		return Lhs;
	}

	friend FCollectedItem operator+=(FCollectedItem& Lhs, const int32 RHS)
	{
		return Lhs + RHS;
	}

	friend FCollectedItem operator-(FCollectedItem& Lhs, FCollectedItem& RHS)
	{
		Lhs.Amount -= RHS.Amount;
		return Lhs;
	}

	friend FCollectedItem operator-=(FCollectedItem& Lhs, FCollectedItem& RHS)
	{
		return Lhs - RHS;
	}

	friend FCollectedItem operator-(FCollectedItem& Lhs, const int32 RHS)
	{
		Lhs.Amount -= RHS;
		return Lhs;
	}

	friend FCollectedItem operator-=(FCollectedItem& Lhs, const int32 RHS)
	{
		return Lhs - RHS;
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
	TArray<FCollectedItem> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	AActor* EquippedItem;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UFUNCTION(BlueprintCallable)
	void Collect(AActor* Item);

	UFUNCTION(BlueprintCallable)
	int32 Get(TSubclassOf<AActor> ItemClass, int32 Amount);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Equip(int32 Index, bool IsValid);

protected:
	UFUNCTION(Server, Reliable)
	void CollectItem(FCollectedItem Item);
	
	UFUNCTION()
	bool CanCollect(AActor* Item) const;
	
public:
	FCollectedItem GetItem(TSubclassOf<AActor> ItemClass) const;
	
	FORCEINLINE AActor* GetEquippedItem() const { return EquippedItem; }
		
};
