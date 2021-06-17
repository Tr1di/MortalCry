// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "MortalCryEditorStyle.h"

class FMortalCryEditorCommands : public TCommands<FMortalCryEditorCommands>
{
public:

	FMortalCryEditorCommands()
		: TCommands<FMortalCryEditorCommands>(TEXT("SimpleUGCEditor"), NSLOCTEXT("Contexts", "SimpleUGCEditor", "SimpleUGCEditor Plugin"), NAME_None, FMortalCryEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

	TArray<TSharedPtr<FUICommandInfo>> RegisterUGCCommands(const TArray<TSharedRef<class IPlugin>>& UGCList) const;
	void UnregisterUGCCommands(TArray<TSharedPtr<FUICommandInfo>>& UICommands) const;

public:
	TSharedPtr< FUICommandInfo > CreateUGCAction;
	TSharedPtr< FUICommandInfo > PackageUGCAction;
};