// Copyright Epic Games, Inc. All Rights Reserved.

#include "MortalCryPluginWizardDefinition.h"
#include "ContentBrowserModule.h"
#include "EngineAnalytics.h"
#include "Interfaces/IPluginManager.h"
#include "IContentBrowserSingleton.h"
#include "Algo/Transform.h"
#include "SlateBasics.h"
#include "SourceCodeNavigation.h"

#define LOCTEXT_NAMESPACE "SimpleUGCPluginWizard"

FMortalCryPluginWizardDefinition::FMortalCryPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("SimpleUGC"))->GetBaseDir();

	PopulateTemplatesSource();
}

void FMortalCryPluginWizardDefinition::PopulateTemplatesSource()
{
	// Find the Content Only Template that ships with the plugin. 
	// Download the Robo Recall Mod Kit and check the Plugins/OdinEditor code for how to build and use your own UGC templates from your game content
	BackingTemplate = MakeShareable(new FPluginTemplateDescription(FText(), FText(), TEXT("BaseTemplate"), true, EHostType::Runtime));
	BackingTemplatePath = PluginBaseDir / TEXT("Templates") / BackingTemplate->OnDiskPath;
	
	FText BaseCodeLabel = LOCTEXT("BaseCode_Label", "Blank Mod (C++)");
	FText BaseCodeDescription = LOCTEXT("BaseCode_Description", "Создание пустого шаблона");
	BaseCodeTemplate = MakeShareable(new FPluginTemplateDescription(BaseCodeLabel, BaseCodeDescription, TEXT("BaseCodeTemplate"), true, EHostType::Runtime));

	FText WeaponCodeName = LOCTEXT("WeaponCode_Label", "Custom Weapon (C++)");
	FText WeaponCodeDescription = LOCTEXT("WeaponCode_Description", "Создание пустого шаблона Weapon");
	//Оружие
	FText NewGunName = LOCTEXT("BaseWeapon_Label", "New Gun");
	FText NewGunDescription = LOCTEXT("BaseWeapon_Description", "Создание нового Weapon");
	TemplateDefinitions.Add(MakeShareable(new FPluginTemplateDescription(NewGunName, NewGunDescription, TEXT("NewGun"), true, EHostType::Runtime)));
	//Коробки с броней
	FText NewAmmoName = LOCTEXT("BaseArmor_Label", "New Ammo");
	FText NewAmmoDescription = LOCTEXT("BaseArmor_Description", "Создание нового Ammo");
	TemplateDefinitions.Add(MakeShareable(new FPluginTemplateDescription(NewAmmoName, NewAmmoDescription, TEXT("NewAmmo"), true, EHostType::Runtime)));

	//Коробки с HP
	FText NewHPName = LOCTEXT("BaseHP_Label", "New HP");
	FText NewHPDescription = LOCTEXT("BaseHP_Description", "Создание нового HP");
	TemplateDefinitions.Add(MakeShareable(new FPluginTemplateDescription(NewHPName, NewHPDescription, TEXT("NewHP"), true, EHostType::Runtime)));

	if (FSourceCodeNavigation::IsCompilerAvailable())
	{
		TemplateDefinitions.Add(BaseCodeTemplate.ToSharedRef());
		TemplateDefinitions.Add(MakeShareable(new FPluginTemplateDescription(WeaponCodeName, WeaponCodeDescription, TEXT("WeaponCodeTemplate"), true, EHostType::Runtime)));
	}
	
	TemplateToIconMap.Add(NewGunName.ToString(), TEXT("PISTOL.png"));
	TemplateToIconMap.Add(NewAmmoName.ToString(), TEXT("AMMO.png"));
	TemplateToIconMap.Add(NewHPName.ToString(), TEXT("HPP.png"));
	
}

const TArray<TSharedRef<FPluginTemplateDescription>>& FMortalCryPluginWizardDefinition::GetTemplatesSource() const
{
	return TemplateDefinitions;
}

void FMortalCryPluginWizardDefinition::OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo)
{
	SelectedTemplates = InSelectedItems;
}

TArray<TSharedPtr<FPluginTemplateDescription>> FMortalCryPluginWizardDefinition::GetSelectedTemplates() const
{
	TArray<TSharedPtr<FPluginTemplateDescription>> SelectedTemplatePtrs;

	for (TSharedRef<FPluginTemplateDescription> Ref : SelectedTemplates)
	{
		SelectedTemplatePtrs.Add(Ref);
	}

	return SelectedTemplatePtrs;
}

void FMortalCryPluginWizardDefinition::ClearTemplateSelection()
{
	SelectedTemplates.Empty();
}

bool FMortalCryPluginWizardDefinition::HasValidTemplateSelection() const
{
	// A mod should be created even if no templates are actually selected
	return true;
}

bool FMortalCryPluginWizardDefinition::CanContainContent() const 
{
	bool bHasContent = SelectedTemplates.Num() == 0;	// if no templates are selected, by default it is a content mod

	if (!bHasContent)
	{
		for (TSharedPtr<FPluginTemplateDescription> Template : SelectedTemplates)
		{
			// If at least one module can contain content, it's a content mod. Otherwise, it's a pure code mod.
			if (Template->bCanContainContent)
			{
				bHasContent = true;
				break;
			}
		}
	}

	return bHasContent;
}

bool FMortalCryPluginWizardDefinition::HasModules() const
{
	bool bHasModules = false;

	for (TSharedPtr<FPluginTemplateDescription> Template : SelectedTemplates)
	{
		if (FPaths::DirectoryExists(PluginBaseDir / TEXT("Templates") / Template->OnDiskPath / TEXT("Source")))
		{
			bHasModules = true;
			break;
		}
	}

	return bHasModules;
}

bool FMortalCryPluginWizardDefinition::IsMod() const
{
	return true;
}

void FMortalCryPluginWizardDefinition::OnShowOnStartupCheckboxChanged(ECheckBoxState CheckBoxState)
{
}

ECheckBoxState FMortalCryPluginWizardDefinition::GetShowOnStartupCheckBoxState() const
{
	return ECheckBoxState();
}

FText FMortalCryPluginWizardDefinition::GetInstructions() const
{
	return LOCTEXT("CreateNewPanel", "Выбери нужный мод и нажмите на кнопку'Create Mod' .");
}

TSharedPtr<SWidget> FMortalCryPluginWizardDefinition::GetCustomHeaderWidget()
{
	if ( !CustomHeaderWidget.IsValid() )
	{
		FString IconPath;
		GetPluginIconPath(IconPath);

		const FName BrushName(*IconPath);
		const FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(BrushName);
		if ((Size.X > 0) && (Size.Y > 0))
		{
			IconBrush = MakeShareable(new FSlateDynamicImageBrush(BrushName, FVector2D(Size.X, Size.Y)));
		}

		CustomHeaderWidget = SNew(SHorizontalBox)
			// Header image
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.0f)
			[
				SNew(SBox)
				.WidthOverride(80.0f)
				.HeightOverride(80.0f)
				[
					SNew(SImage)
					.Image(IconBrush.IsValid() ? IconBrush.Get() : nullptr)
				]
			];
	}

	return CustomHeaderWidget;
}

bool FMortalCryPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	// Replace this file with your own 128x128 image if desired.
	OutIconPath = BackingTemplatePath / TEXT("Resources/Icon128.png");
	return false;
}

bool FMortalCryPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	FString TemplateName = InTemplate->Name.ToString();

	OutIconPath = PluginBaseDir / TEXT("Resources");

	if (TemplateToIconMap.Contains(TemplateName))
	{
		OutIconPath /= TemplateToIconMap[TemplateName];
	}
	else
	{
		// Couldn't find a suitable icon to use for this template, so use the default one instead
		OutIconPath /= TEXT("Icon128.png");
	}
	
	return false;
}

FString FMortalCryPluginWizardDefinition::GetPluginFolderPath() const
{
	return BackingTemplatePath;
}

EHostType::Type FMortalCryPluginWizardDefinition::GetPluginModuleDescriptor() const
{
	return BackingTemplate->ModuleDescriptorType;
}

ELoadingPhase::Type FMortalCryPluginWizardDefinition::GetPluginLoadingPhase() const
{
	return BackingTemplate->LoadingPhase;
}

TArray<FString> FMortalCryPluginWizardDefinition::GetFoldersForSelection() const
{
	TArray<FString> SelectedFolders;
	SelectedFolders.Add(BackingTemplatePath);	// This will always be a part of the mod plugin

	for (TSharedPtr<FPluginTemplateDescription> Template : SelectedTemplates)
	{
		SelectedFolders.AddUnique(PluginBaseDir / TEXT("Templates") / Template->OnDiskPath);
	}

	return SelectedFolders;
}

void FMortalCryPluginWizardDefinition::PluginCreated(const FString& PluginName, bool bWasSuccessful) const
{
	// Override Category to UGC
	if (bWasSuccessful)
	{
		TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(PluginName);
		if (Plugin != nullptr)
		{
			FPluginDescriptor Desc = Plugin->GetDescriptor();
			Desc.Category = "UGC";
			FText UpdateFailureText;
			Plugin->UpdateDescriptor(Desc, UpdateFailureText);
		}
	}
}

#undef LOCTEXT_NAMESPACE
