// Copyright Epic Games, Inc. All Rights Reserved.

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
                "WorkspaceMenuStructure"
                // ... add private dependencies that you statically link with here ...	
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );


        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        //PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Public\\libplateau"));

        //using c++17
        CppStandard = CppStandardVersion.Cpp17;
    }
}
