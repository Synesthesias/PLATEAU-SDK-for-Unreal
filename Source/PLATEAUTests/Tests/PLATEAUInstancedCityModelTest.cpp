// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/GameplayStatics.h"


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUInstancedCityModelTest, FPLATEAUAutomationTestBase, "PLATEAUTest.InstancedCityModel",
                                        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUInstancedCityModelTest::RunTest(const FString& Parameters) {
    if (!OpenNewMap()) AddError("Failed to OpenNewMap");

    const auto& Loader = GetInstancedCityLoader(*GetWorld());
    Loader->LoadAsync(true);

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Loader] {
        if (Loader->Phase == ECityModelLoadingPhase::Cancelling || Loader->Phase == ECityModelLoadingPhase::Finished) {
            TArray<AActor*> CityModelActors;
            UGameplayStatics::GetAllActorsOfClass(Loader->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModelActors);
            if (CityModelActors.Num() <= 0) AddError("CityModelActors.Num() <= 0");
            
            return true;
         }
         return false;
    }));

    return true;
}
