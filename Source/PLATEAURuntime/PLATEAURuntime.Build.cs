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
                Path.Combine(ModuleDirectory, "Public/libplateau")
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
                "Core", "CoreUObject", "Engine", "InputCore"
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
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

        string libPath = Path.Combine(ModuleDirectory, "Private");
        PublicLibraryPaths.Add(libPath);
        PublicAdditionalLibraries.Add("plateau.lib");
        PublicAdditionalLibraries.Add("citygml.lib");

        string dllPath = Path.Combine(libPath, "plateau.dll");
        string dllName = "plateau.dll";
        CopyDll(dllName, dllPath);
        PublicDelayLoadDLLs.Add(dllName);
        RuntimeDependencies.Add(dllPath);

        dllPath = Path.Combine(libPath, "citygml.dll");
        dllName = "citygml.dll";
        CopyDll(dllName, dllPath);
        PublicDelayLoadDLLs.Add(dllName);
        RuntimeDependencies.Add(dllPath);
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
