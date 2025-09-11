//
// Aether: Real-Time Sky & Environment & Weather simulation plugin.
//		Copyright Technical Artist - Jiahao.Chan, Individual. All Rights Reserved.
//

using UnrealBuildTool;

public class Aether : ModuleRules
{
	public Aether(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				ModuleDirectory + "/Public",
				ModuleDirectory + "/Public/Avatar",
				ModuleDirectory + "/Public/Controller",
				ModuleDirectory + "/Public/WeatherEvent",
				ModuleDirectory + "/Private",
				ModuleDirectory + "/Private/Avatar",
				ModuleDirectory + "/Private/Controller",
				ModuleDirectory + "/Private/WeatherEvent",
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
				//"Renderer",
				//"RenderCore",
				//"RHI",
			}
			);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"DeveloperSettings",
				"GameplayTags",
				"Niagara",
				"Projects",
				"Renderer",
				"RenderCore",
				"RHI",
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
