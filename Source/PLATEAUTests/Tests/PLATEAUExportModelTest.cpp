// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUCityModelLoader.h"
#include "Export/PLATEAUExportModelAPI.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUExportSettings.h"
#include "HAL/FileManagerGeneric.h"
#include "Kismet/GameplayStatics.h"


namespace {
    TArray<FString> FindFiles(const FString& Path, const FString& Filter) {
        TArray<FString> FoundFileArray;
        FoundFileArray.Empty();

        if (FPaths::DirectoryExists(Path)) {
            if (0 < Filter.Len()) {
                if (Path.Right(1) == "/") {
                    FFileManagerGeneric::Get().FindFiles(FoundFileArray, *(Path + Filter), true, false);
                } else {
                    FFileManagerGeneric::Get().FindFiles(FoundFileArray, *(Path + "/" + Filter), true, false);
                }
            }
            FFileManagerGeneric::Get().FindFiles(FoundFileArray, *Path, true, false);
        }

        return FoundFileArray;
    }
}


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_ModelExporter_Export_Generates_Files, FPLATEAUAutomationTestBase,
                                        "PLATEAUTest.FPLATEAUTest.ModelExporter.Export_Generates_Files",
                                        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_ModelExporter_Export_Generates_Files::RunTest(const FString& Parameters) {
    InitializeTest("Export_Generates_Files");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    const FString TestDir = FPaths::ProjectDir().Append("Tests");
    if (FPaths::DirectoryExists(TestDir)) {
        if (!FFileManagerGeneric::Get().DeleteDirectory(*TestDir, true, true))
            AddError("Failed to DeleteDirectory");
    }
    
    const auto& Loader = GetInstancedCityLoader(*GetWorld());
    Loader->LoadAsync(true);

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Loader, TestDir] {
        if (Loader->Phase != ECityModelLoadingPhase::Cancelling && Loader->Phase != ECityModelLoadingPhase::Finished)
            return false;

        TArray<AActor*> CityModelActors;
        UGameplayStatics::GetAllActorsOfClass(Loader->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModelActors);
        if (CityModelActors.Num() <= 0) {
            FinishTest(false, "CityModelActors.Num() <= 0");
            return true;
        }

        if (!FFileManagerGeneric::Get().MakeDirectory(*TestDir, true)) {
            FinishTest(false, "Failed to MakeDirectory");
            return true;
        }

        FPLATEAUMeshExportOptions Options;
        Options.FileFormat = EMeshFileFormat::FBX;
        Options.bExportAsBinary = false;
        Options.bExportHiddenObjects = true;
        Options.bExportTexture = true;
        Options.TransformType = EMeshTransformType::Local;
        Options.CoordinateSystem = ECoordinateSystem::ENU;

        UPLATEAUExportModelAPI::ExportModel(Cast<APLATEAUInstancedCityModel>(CityModelActors.GetData()[0]), TestDir, Options);
        const auto FoundFiles = FindFiles(TestDir, "*");
        if (FoundFiles.Num() <= 0) {
            FinishTest(false, "Failed to FindFiles");
            return true;
        }

        FinishTest(true, "");
        return true;
    }));

    return true;
}