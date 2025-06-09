// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EasySynchronizer : ModuleRules
{
	public EasySynchronizer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"Public/EasySynchronizer/"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Core",
				"GameplayTags"
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd"
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);

	}
}
