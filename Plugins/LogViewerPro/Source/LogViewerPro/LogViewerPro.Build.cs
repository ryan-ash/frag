// Copyright Dmitrii Labadin 2019

using UnrealBuildTool;

public class LogViewerPro : ModuleRules
{
	public LogViewerPro(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
                //"Core",
                "InputCore",
				"UnrealEd",
				"LevelEditor",
				"CoreUObject",
                "Engine",
				"Slate",
				"SlateCore",
                "EditorStyle",
                "TargetPlatform",
                "DesktopPlatform",
                "ApplicationCore",
				"Json",
                "AppFramework"
				// ... add private dependencies that you statically link with here ...	
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
