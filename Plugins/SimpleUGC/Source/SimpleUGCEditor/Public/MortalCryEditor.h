// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Slate.h"
#include "MortalCryPackager.h"
#include "Modules\ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FMortalCryEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	// When the Create Button is clicked
	void CreateUGCButtonClicked();

	/** Adds the plugin creator as a new toolbar button */
	void AddUGCCreatorToolbarExtension(FToolBarBuilder& Builder);

	/** Adds the plugin creator as a new menu option */
	void AddUGCCreatorMenuExtension(FMenuBuilder& Builder);

	/** Adds the plugin packager as a new toolbar button */
	void AddUGCPackagerToolbarExtension(FToolBarBuilder& Builder);

	/** Adds the plugin packager as a new menu option */
	void AddUGCPackagerMenuExtension(FMenuBuilder& Builder);
	
private:

	TSharedPtr<class FMortalCryCreator> UGCCreator;
	TSharedPtr<class FMortalCryPackager> UGCPackager;
	TSharedPtr<class FUICommandList> PluginCommands;
};