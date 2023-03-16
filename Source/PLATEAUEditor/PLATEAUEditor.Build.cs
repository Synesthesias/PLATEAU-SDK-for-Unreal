// Copyright © 2023 Ministry of Land、Infrastructure and Transport

using UnrealBuildTool;
using System;
using System.IO;

public class PLATEAUEditor : ModuleRules
{
    public PLATEAUEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "PLATEAURuntime"
                // ... add other public dependencies that you statically link with here ...
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "LevelEditor",
                "Projects",
                "DesktopPlatform",
                "InputCore",
                "FBX",
                "UnrealEd",
                "AssetTools",
                "EditorStyle",
                "PropertyEditor",
                "AdvancedPreviewScene",
                "WorkspaceMenuStructure",
                "MeshDescription",
                "StaticMeshDescription",
                "RHI"
                // ... add private dependencies that you statically link with here ...	
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );


        var plateauLibs = new string[]
        {
            "plateau"
        };

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/include"));

        PublicAdditionalLibraries.Add("glu32.lib");
        PublicAdditionalLibraries.Add("opengl32.lib");

        foreach (var lib in plateauLibs)
        {
            var libPath = Path.Combine(ModuleDirectory, "../ThirdParty/lib");
            PublicAdditionalLibraries.Add(Path.Combine(libPath, $"{lib}.lib"));
        }

        PublicDefinitions.Add("CITYGML_STATIC_DEFINE");

        //using c++17
        CppStandard = CppStandardVersion.Cpp17;
    }
}
