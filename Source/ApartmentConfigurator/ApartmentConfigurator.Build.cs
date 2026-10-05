using UnrealBuildTool;

public class ApartmentConfigurator : ModuleRules
{
	public ApartmentConfigurator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			// JSON parsing for the building configuration.
			"Json",
			"JsonUtilities",
			// UMG runtime UI.
			"UMG",
			"Slate",
			"SlateCore"
		});
	}
}
