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
	FBox2D Place;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> Item;
	
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
	TMap<TSubclassOf<AActor>, int32> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Inventory, meta = (AllowPrivateAccess = "True"))
	AActor* EquippedItem;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void Collect(AActor* Item);
	
protected:
	UFUNCTION()
	bool CanCollect(AActor* Item) const;
	
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE AActor* GetEquippedItem() const { return EquippedItem; }
		
};
