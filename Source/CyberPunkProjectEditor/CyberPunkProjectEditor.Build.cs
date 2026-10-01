// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CyberPunkProjectEditor : ModuleRules
{
	public CyberPunkProjectEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"CyberPunkProject"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"Blutility",
			"AssetTools",
			"MaterialEditor",
			"Slate",
			"SlateCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"CyberPunkProjectEditor/Public"
		});

		PrivateIncludePaths.AddRange(new string[] {
			"CyberPunkProjectEditor/Private"
		});
	}
}
