// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), BlueprintType, meta=(BlueprintSpawnableComponent) )
class MORTALCRY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Health, meta = (AllowPrivateAccess="True"))
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Health, meta = (AllowPrivateAccess="True"))
    float Health;
    
public:	
    // Sets default values for this component's properties
    explicit UHealthComponent(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintCallable, Category=Health)
    void Update(float HealthChange);

    UFUNCTION(BlueprintPure, Category=Health)
    float GetHealth() const;

    UFUNCTION(BlueprintPure, Category=Health)
    bool IsAlive() const;

		
};
