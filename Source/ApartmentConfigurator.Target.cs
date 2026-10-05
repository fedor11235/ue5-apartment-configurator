using UnrealBuildTool;
using System.Collections.Generic;

public class ApartmentConfiguratorTarget : TargetRules
{
	public ApartmentConfiguratorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ApartmentConfigurator");
	}
}
