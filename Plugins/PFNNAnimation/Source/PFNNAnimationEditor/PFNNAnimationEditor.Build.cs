using UnrealBuildTool;

public class PFNNAnimationEditor : ModuleRules
{
    public PFNNAnimationEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        //Public module names that this module uses.
        //In case you would like to add various classes that you're going to use in your game
        //you should add the core,coreuobject and engine dependencies.
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "PFNNAnimation", "AnimGraph", "BlueprintGraph"});
    }
}