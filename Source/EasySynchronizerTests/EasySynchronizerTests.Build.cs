using UnrealBuildTool;

public class EasySynchronizerTests : ModuleRules
{
    public EasySynchronizerTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "EasySynchronizer",
                "GameplayTags",
                "UnrealEd"
            }
        );
    }
}