// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CyberPunkProject : ModuleRules
{
	public CyberPunkProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"CyberPunkProject/Public",
			"CyberPunkProject/Public/NeonDistrict",
			"CyberPunkProject/Public/Variant_Shooter",
			"CyberPunkProject/Public/Variant_Shooter/AI",
			"CyberPunkProject/Public/Variant_Shooter/UI",
			"CyberPunkProject/Public/Variant_Shooter/Weapons"
		});

		PrivateIncludePaths.AddRange(new string[] {
			"CyberPunkProject/Private"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
