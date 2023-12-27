// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/GameplayStatics.h"
#include "Tests/AutomationCommon.h"


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_CityModelLoader_Load_Generates_Actor, FPLATEAUAutomationTestBase, "PLATEAUTest.FPLATEAUTest.CityModelLoader.Load_Generates_Actor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_CityModelLoader_Load_Generates_Actor::RunTest(const FString& Parameters) {
    InitializeTest("Load_Generates_Actor");
    if (!OpenNewMap())
        AddError("Failed to OpenNewMap");

    const auto& Loader = GetInstancedCityLoader(*GetWorld());
    Loader->LoadAsync(true);

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Loader] {
        if (Loader->Phase != ECityModelLoadingPhase::Cancelling && Loader->Phase != ECityModelLoadingPhase::Finished)
            return false;

        TArray<AActor*> CityModelActors;
        UGameplayStatics::GetAllActorsOfClass(Loader->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModelActors);
        if (CityModelActors.Num() <= 0) {
            FinishTest(false, "CityModelActors.Num() <= 0");
            return true;
        }

        FinishTest(true, "");
        return true;
    }));

    return true;
}
