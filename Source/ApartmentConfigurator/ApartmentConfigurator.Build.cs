using UnrealBuildTool;

public class ApartmentConfigurator : ModuleRules
{
	public ApartmentConfigurator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Sources live directly under the module root (no Public/Private split),
		// and headers include each other module-root-relative (e.g. "Data/ConfiguratorTypes.h").
		// Expose the module root so those includes resolve.
		PublicIncludePaths.Add(ModuleDirectory);

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
