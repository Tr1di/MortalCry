// Copyright Epic Games, Inc. All Rights Reserved.

//#include "SimpleUGCEditorPrivatePCH.h"
#include "MortalCryCreator.h"

#include "MortalCryPluginWizardDefinition.h"
#include "Widgets/Docking/SDockTab.h"

// This depends on the Plugin Browser module to work correctly...
#include "IPluginBrowser.h"



#define LOCTEXT_NAMESPACE "FSimpleUGCCreator"

const FName FMortalCryCreator::SimpleUGCEditorPluginCreatorName("SimpleUGCPluginCreator");

FMortalCryCreator::FMortalCryCreator()
{
	RegisterTabSpawner();
}

FMortalCryCreator::~FMortalCryCreator()
{
	UnregisterTabSpawner();
}

void FMortalCryCreator::OpenNewPluginWizard(bool bSuppressErrors) const
{
	if (IPluginBrowser::IsAvailable())
	{
		FGlobalTabmanager::Get()->InvokeTab(SimpleUGCEditorPluginCreatorName);
	}
	else if (!bSuppressErrors)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("PluginBrowserDisabled", "Creating a game mod requires the use of the Plugin Browser, but it is currently disabled."));
	}
}

void FMortalCryCreator::RegisterTabSpawner()
{
	FTabSpawnerEntry& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SimpleUGCEditorPluginCreatorName,
																FOnSpawnTab::CreateRaw(this, &FMortalCryCreator::HandleSpawnPluginTab));

	// Set a default size for this tab
	FVector2D DefaultSize(800.0f, 500.0f);
	FTabManager::RegisterDefaultTabWindowSize(SimpleUGCEditorPluginCreatorName, DefaultSize);

	Spawner.SetDisplayName(LOCTEXT("NewUGCTabHeader", "Создание нового контента"));
	Spawner.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FMortalCryCreator::UnregisterTabSpawner()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SimpleUGCEditorPluginCreatorName);
}

TSharedRef<SDockTab> FMortalCryCreator::HandleSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	check(IPluginBrowser::IsAvailable());
	return IPluginBrowser::Get().SpawnPluginCreatorTab(SpawnTabArgs, MakeShared<FMortalCryPluginWizardDefinition>());
}

#undef LOCTEXT_NAMESPACE