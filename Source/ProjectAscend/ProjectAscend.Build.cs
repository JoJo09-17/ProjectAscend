using UnrealBuildTool;

public class ProjectAscend : ModuleRules
{
	public ProjectAscend(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.AddRange(
			new string[] {
				"ProjectAscend"
			}
		);
		
		PrivateIncludePaths.AddRange(
			new string[] {
				""
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreOnline",
				"CoreUObject",
				"ApplicationCore",
				"Engine",
				"PhysicsCore",
				"GameplayTags",
				"GameplayTasks",
				"GameplayAbilities",
				"AIModule",
				"DataRegistry",
				"Niagara",
				"EnhancedInput",
				"NetCore",
				"UMG",
				"Slate",
				"SlateCore",
				"MotionWarping",
				"StructUtils",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"RenderCore",
				"DeveloperSettings",
				"Projects",
				"AnimGraphRuntime",
				"ModelViewViewModel",
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
			}
		);
	}
}
