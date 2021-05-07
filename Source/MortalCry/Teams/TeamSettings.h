// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GenericTeamAgentInterface.h"
#include "Engine/DeveloperSettings.h"

#include "TeamSettings.generated.h"

USTRUCT(BlueprintType)
struct FTeamAttitude
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<TEnumAsByte<ETeamAttitude::Type>> Attitude;

	FTeamAttitude()
	{}

	explicit FTeamAttitude(const TArray<TEnumAsByte<ETeamAttitude::Type>> Attitudes) : Attitude(Attitudes)
	{}

	FTeamAttitude(std::initializer_list<TEnumAsByte<ETeamAttitude::Type>> Attitudes):
		Attitude(std::move(Attitudes))
	{}
};

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig)
class MORTALCRY_API UTeamSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Teams")
	TArray<FTeamAttitude> TeamAttitudes;
	
	explicit UTeamSettings(const FObjectInitializer& ObjectInitializer);
	
	static const UTeamSettings* Get();

	UFUNCTION(BlueprintPure, Category = "Teams")
	static ETeamAttitude::Type GetAttitude(FGenericTeamId Of, FGenericTeamId Towards);
};
