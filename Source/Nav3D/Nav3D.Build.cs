// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Nav3D : ModuleRules
{
	public Nav3D(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bUseUnity = true;
		
		// Enable strict validation to catch issues like missing Category specifiers
		bTreatAsEngineModule = true; // Treat plugin as Engine module for stricter validation
		
		PublicIncludePaths.AddRange(
			new string[]
			{
				ModuleDirectory + "/../ThirdParty"
			}
		);
		
		PrivateIncludePaths.AddRange(
			new[]
			{
				"Nav3D/Private"
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core", "AIModule", 
				"FishyUtils"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"RHI",
				"RenderCore",
				"DeveloperSettings",
				"GameplayTasks",
				"AIModule",
				"NavigationSystem",
				"Landscape",
				"InputCore",
			}
		);
		
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.Add("UnrealEd");
		}
	}
}