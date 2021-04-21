// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryHUD.h"

#include "CanvasItem.h"
#include "Informative.h"
#include "MortalCryCharacter.h"
#include "MortalCryPlayerController.h"
#include "TextureResource.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

AMortalCryHUD::AMortalCryHUD()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPerson/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}

void AMortalCryHUD::DrawHUD()
{
	Super::DrawHUD();

	DrawCrossHair();
	DrawInteractionText();
}

void AMortalCryHUD::DrawInteractionText_Implementation()
{
	AActor* InteractiveActor = nullptr;

	if ( AMortalCryPlayerController* PC = Cast<AMortalCryPlayerController>(GetOwningPlayerController()) )
	{
		InteractiveActor = PC->Trace();
	}

	if ( InteractiveActor && InteractiveActor->Implements<UInformative>() )
	{
		const FString Text = IInformative::Execute_GetName(InteractiveActor);
		const FVector ProjectVector = Project(InteractiveActor->GetActorLocation());

		if ( ProjectVector.Z > 0 )
		{
			float Width, Height;
			GetTextSize(Text, Width, Height);

			Width /= 2;
			DrawText(Text, FLinearColor::White, ProjectVector.X, ProjectVector.Y - Height);
		}
	}
}

void AMortalCryHUD::DrawCrossHair_Implementation()
{
	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X),
                                           (Center.Y + 20.0f));

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}