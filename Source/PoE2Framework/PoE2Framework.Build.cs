using UnrealBuildTool;

public class PoE2Framework : ModuleRules
{
    public PoE2Framework(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks",
                "AIModule",
                "NavigationSystem"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "NetCore",
                "ReplicationGraph"
            }
        );
        // This block is ESSENTIAL for Automation Tests to be discovered.
        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "UnrealEd",
                "ToolMenus",
                "AutomationDriver"
            });
        }
    }
}