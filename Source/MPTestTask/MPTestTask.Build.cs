// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MPTestTask : ModuleRules
{
	public MPTestTask(ReadOnlyTargetRules Target) : base(Target)
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
			"MPTestTask",
			"MPTestTask/Variant_Platforming",
			"MPTestTask/Variant_Platforming/Animation",
			"MPTestTask/Variant_Combat",
			"MPTestTask/Variant_Combat/AI",
			"MPTestTask/Variant_Combat/Animation",
			"MPTestTask/Variant_Combat/Gameplay",
			"MPTestTask/Variant_Combat/Interfaces",
			"MPTestTask/Variant_Combat/UI",
			"MPTestTask/Variant_SideScrolling",
			"MPTestTask/Variant_SideScrolling/AI",
			"MPTestTask/Variant_SideScrolling/Gameplay",
			"MPTestTask/Variant_SideScrolling/Interfaces",
			"MPTestTask/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
