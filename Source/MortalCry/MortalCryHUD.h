// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"

#include "GameFramework/HUD.h"

#include "MortalCryHUD.generated.h"

UCLASS()
class AMortalCryHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMortalCryHUD();

private:
	/** Crosshair asset pointer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UTexture2D* CrosshairTex;

protected:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DrawInteractionText();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DrawCrossHair();

public:
	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

};

