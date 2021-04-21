// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MortalCryEditorTarget : TargetRules
{
	public MortalCryEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("MortalCry");
	}
}
