using UnrealBuildTool;

public class PFNNAnimation : ModuleRules
{
    public PFNNAnimation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AnimGraph", "BlueprintGraph"});
    }
}