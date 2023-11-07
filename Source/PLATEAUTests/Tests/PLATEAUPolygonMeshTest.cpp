// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "EngineUtils.h"
#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/GameplayStatics.h"


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_CityModelLoader_Load_Generates_Components, FPLATEAUAutomationTestBase,
                                        "PLATEAUTest.FPLATEAUTest.CityModelLoader.Load_Generates_Components",
                                        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_CityModelLoader_Load_Generates_Components::RunTest(const FString& Parameters) {
    InitializeTest("Load_Generates_Components");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    const auto& Loader = GetInstancedCityLoader(*GetWorld());
    Loader->LoadAsync(true);

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Loader] {
        if (Loader->Phase != ECityModelLoadingPhase::Cancelling && Loader->Phase != ECityModelLoadingPhase::Finished)
            return false;
            
        bool bExistPolygonMesh = false;
        TArray<AActor*> CityModelActors;
        UGameplayStatics::GetAllActorsOfClass(Loader->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModelActors);
        if (CityModelActors.Num() <= 0) {
            FinishTest(false, "CityModelActors.Num() <= 0");
            return true;
        }

        for (const auto& CityModelActor : CityModelActors) {
            TArray<USceneComponent*> GmlActors;
            CityModelActor->GetRootComponent()->GetChildrenComponents(false, GmlActors);
            if (GmlActors.Num() <= 0) {
                FinishTest(false, "GmlActors.Num() <= 0");
                return true;
            }
            
            for (const auto& GmlActor : GmlActors) {
                TArray<USceneComponent*> LodActors;
                GmlActor->GetChildrenComponents(false, LodActors);
                
                for (const auto& LodActor : LodActors) {
                    if (LodActor->GetName().Contains("LOD", ESearchCase::CaseSensitive)) {
                        bExistPolygonMesh = true;
                    }
                }
            }
        }

        if (!bExistPolygonMesh) {
            FinishTest(false, "bExistPolygonMesh == false");
            return true;
        }
        
        FinishTest(true, "");
        return true;
    }));

    return true;
}