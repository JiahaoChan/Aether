//
// Aether: Real-Time Sky & Environment & Weather simulation plugin.
//		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
//

using UnrealBuildTool;

public class AetherEditor : ModuleRules
{
	public AetherEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Public",
				ModuleDirectory + "/Private",
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
				"Engine",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Aether",
				"CoreUObject",
				"Slate",
				"SlateCore",
				"UnrealEd",
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
