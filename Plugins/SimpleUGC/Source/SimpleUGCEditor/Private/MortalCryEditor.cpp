// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryEditor.h"
#include "MortalCryEditorStyle.h"
#include "MortalCryEditorCommands.h"
#include "MortalCryCreator.h"
#include "Misc/MessageDialog.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "LevelEditor.h"

static const FName SimpleUGCEditorTabName("SimpleUGCEditor");

#define LOCTEXT_NAMESPACE "FSimpleUGCEditorModule"

void FMortalCryEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	UGCCreator = MakeShared<FMortalCryCreator>();
	UGCPackager = MakeShared<FMortalCryPackager>();

	FMortalCryEditorStyle::Initialize();
	FMortalCryEditorStyle::ReloadTextures();

	FMortalCryEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FMortalCryEditorCommands::Get().CreateUGCAction,
		FExecuteAction::CreateRaw(this, &FMortalCryEditorModule::CreateUGCButtonClicked),
		FCanExecuteAction()
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	// Add commands
	{
		FName MenuSection = "FileProject";
		FName ToolbarSection = "Misc";

		// Add creator button to the menu
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension(MenuSection, EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FMortalCryEditorModule::AddUGCCreatorMenuExtension));

			LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}

		// Add creator button to the toolbar
		{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension(ToolbarSection, EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FMortalCryEditorModule::AddUGCCreatorToolbarExtension));

			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}

		// Add packager button to the menu
		{
			TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
			MenuExtender->AddMenuExtension(MenuSection, EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FMortalCryEditorModule::AddUGCPackagerMenuExtension));

			LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}

		// Add packager button to the toolbar
		{
			TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
			ToolbarExtender->AddToolBarExtension(ToolbarSection, EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FMortalCryEditorModule::AddUGCPackagerToolbarExtension));

			LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
		}
	}
}

void FMortalCryEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FMortalCryEditorStyle::Shutdown();

	FMortalCryEditorCommands::Unregister();

}

void FMortalCryEditorModule::CreateUGCButtonClicked()
{
	if (UGCCreator.IsValid())
	{
		UGCCreator->OpenNewPluginWizard();
	}
}

void FMortalCryEditorModule::AddUGCCreatorMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FMortalCryEditorCommands::Get().CreateUGCAction);
}

void FMortalCryEditorModule::AddUGCCreatorToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FMortalCryEditorCommands::Get().CreateUGCAction);
}

void FMortalCryEditorModule::AddUGCPackagerMenuExtension(FMenuBuilder& Builder)
{
	FMortalCryPackager* Packager = UGCPackager.Get();

	Builder.AddSubMenu(LOCTEXT("PackageUGCMenu_Label", "Package"),
		LOCTEXT("PackageUGCMenu_Tooltip", "Share and distribute"),
		FNewMenuDelegate::CreateRaw(Packager, &FMortalCryPackager::GeneratePackagerMenuContent),
		false,
		FSlateIcon(FMortalCryEditorStyle::GetStyleSetName(), "SimpleUGCEditor.PackageUGCAction")
	);
}

void FMortalCryEditorModule::AddUGCPackagerToolbarExtension(FToolBarBuilder& Builder)
{
	FMortalCryPackager* Packager = UGCPackager.Get();

	Builder.AddComboButton(FUIAction(),
		FOnGetContent::CreateSP(Packager, &FMortalCryPackager::GeneratePackagerComboButtonContent),
		LOCTEXT("PackageUGC_Label", "Package"),
		LOCTEXT("PackageUGC_Tooltip", "Share and distribute"),
		FSlateIcon(FMortalCryEditorStyle::GetStyleSetName(), "SimpleUGCEditor.PackageUGCAction")
	);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMortalCryEditorModule, SimpleUGCEditor)
