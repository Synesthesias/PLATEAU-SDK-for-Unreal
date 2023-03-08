using UnrealBuildTool;
using System;
using System.IO;

public class PLATEAURuntime : ModuleRules
{
    public PLATEAURuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp17;

        PublicIncludePaths.AddRange(
            new string[]
            {
            }
        );


        PrivateIncludePaths.AddRange(
            new string[]
            {
                // ... add other private include paths required here ...
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", "CoreUObject", "Engine", "InputCore",
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "MeshDescription",
                "StaticMeshDescription",
                "RHI",
                "ImageWrapper",
                "RenderCore",
                "OpenGL"
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
            "plateau",
            "citygml",
            "GLTFSDK",
            "libssl-1_1-x64",
            "xerces-c_4",
            "libcrypto-1_1-x64",
            "libfbxsdk-md",
            "libxml2-md",
            "zlib-md"
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
    }
}
