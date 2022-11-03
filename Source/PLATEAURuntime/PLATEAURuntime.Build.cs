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
            new string[] {
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
                "RenderCore"
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
        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "../ThirdParty/include"));

        string libPath = Path.Combine(ModuleDirectory, "../ThirdParty/lib");
        PublicAdditionalLibraries.Add(Path.Combine(libPath, "plateau.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(libPath, "citygml.lib"));

        string dllPath = Path.Combine(libPath, "plateau.dll");
        string dllName = "plateau.dll";
        CopyDll(dllName, dllPath);

        dllPath = Path.Combine(libPath, "citygml.dll");
        dllName = "citygml.dll";
        CopyDll(dllName, dllPath);

        RuntimeDependencies.Add("$(TargetOutputDir)/plateau.dll", Path.Combine(ModuleDirectory, "../ThirdParty/lib/plateau.dll"));

        RuntimeDependencies.Add("$(TargetOutputDir)/citygml.dll", Path.Combine(ModuleDirectory, "../ThirdParty/lib/citygml.dll"));

        PublicDelayLoadDLLs.Add("plateau.dll");
        PublicDelayLoadDLLs.Add("citygml.dll");

        //using c++17
        CppStandard = CppStandardVersion.Cpp17;
    }

    // copy dll file to Binaries
    private void CopyDll(string dllName, string dllFullPath)
    {
        if (!File.Exists(dllFullPath))
        {
            Console.WriteLine("file {0} does not exist", dllName);
            return;
        }

        string binariesDir = Path.Combine(ModuleDirectory, "../../../../Binaries/Win64/");
        if (!Directory.Exists(binariesDir))
        {
            Directory.CreateDirectory(binariesDir);
        }

        string binariesDllFullPath = Path.Combine(binariesDir, dllName);
        if (File.Exists(binariesDllFullPath))
        {
            File.SetAttributes(binariesDllFullPath, File.GetAttributes(binariesDllFullPath) & ~FileAttributes.ReadOnly);
        }
        
        try
        {
            File.Copy(dllFullPath, binariesDllFullPath, true);
        }
        catch (Exception ex)
        {
            Console.WriteLine("failed to copy file: {0}", dllName);
        }
    }
}
