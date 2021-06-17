// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryEditorCommands.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSimpleUGCEditorModule"

void FMortalCryEditorCommands::RegisterCommands()
{
	UI_COMMAND(CreateUGCAction, "Mortal Cry Editor", "Creating new content for 'Mortal Cry'", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PackageUGCAction, "Package UGC", "Share and distribute your UGC", EUserInterfaceActionType::Button, FInputGesture());
}

TArray<TSharedPtr<FUICommandInfo>> FMortalCryEditorCommands::RegisterUGCCommands(const TArray<TSharedRef<class IPlugin>>& UGCList) const
{
	TArray<TSharedPtr<FUICommandInfo>> AvailableUGCActions;
	AvailableUGCActions.Reserve(UGCList.Num());

	FMortalCryEditorCommands* MutableThis = const_cast<FMortalCryEditorCommands*>(this);

	for (int32 Index = 0; Index < UGCList.Num(); ++Index)
	{
		AvailableUGCActions.Add(TSharedPtr<FUICommandInfo>());
		TSharedRef<IPlugin> UGC = UGCList[Index];

		FString CommandName = "UGCEditorUGC_" + UGC->GetName();

		FUICommandInfo::MakeCommandInfo(MutableThis->AsShared(),
			AvailableUGCActions[Index],
			FName(*CommandName),
			FText::FromString(UGC->GetName()),
			FText::FromString(UGC->GetBaseDir()),
			FSlateIcon(),
			EUserInterfaceActionType::Button,
			FInputGesture());
	}

	return AvailableUGCActions;
}

void FMortalCryEditorCommands::UnregisterUGCCommands(TArray<TSharedPtr<FUICommandInfo>>& UICommands) const
{
	FMortalCryEditorCommands* MutableThis = const_cast<FMortalCryEditorCommands*>(this);

	for (TSharedPtr<FUICommandInfo> Command : UICommands)
	{
		FUICommandInfo::UnregisterCommandInfo(MutableThis->AsShared(), Command.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE