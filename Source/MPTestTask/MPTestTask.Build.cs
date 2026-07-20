// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MPTestTask : ModuleRules
{
	public MPTestTask(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"AIModule",
			"Core",
			"CoreUObject",
			"DeveloperSettings",
			"Engine",
			"EnhancedInput",
			"GameplayStateTreeModule",
			"InputCore",
			"Niagara",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Slate",
			"SlateCore",
			"StateTreeModule",
			"UMG"
		});

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
	}
}
