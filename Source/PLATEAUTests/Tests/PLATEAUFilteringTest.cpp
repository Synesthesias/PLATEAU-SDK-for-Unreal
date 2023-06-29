// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "FileHelpers.h"
#include "PLATEAUAutomationTestBase.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUInstancedCityModel.h"
#include "PLATEAUModelAdjustmentBuilding.h"
#include "PLATEAUModelAdjustmentFilter.h"
#include "Kismet/GameplayStatics.h"


namespace {
    void ApplyFilter(APLATEAUInstancedCityModel* CityModel, plateau::dataset::PredefinedCityModelPackage CityModelPackages, bool bShowModels) {
        for (const auto Package : UPLATEAUImportSettings::GetAllPackages()) {
            if ((Package & CityModelPackages) == plateau::dataset::PredefinedCityModelPackage::None)
                continue;
            if (Package != plateau::dataset::PredefinedCityModelPackage::Building)
                continue;

            int64 CityObjectType = 0;
            for (const auto BuildingSettingFlag : UPLATEAUModelAdjustmentBuilding::GetAllBuildingSettingFlags()) {
                CityObjectType |= BuildingSettingFlag;
            }

            int64 EnablePackage = bShowModels ? static_cast<int64>(Package) : 0;
            TMap<int64, FPLATEAUPackageLod> PackageToLodRangeMap;
            const auto [MinLod, MaxLod] = CityModel->GetMinMaxLod(Package);
            PackageToLodRangeMap.Add(static_cast<int64>(Package), FPLATEAUPackageLod(MinLod, MaxLod));
            UPLATEAUModelAdjustmentFilter::ApplyFilter(CityModel, EnablePackage, PackageToLodRangeMap, true, CityObjectType);
        }
    }
}


IMPLEMENT_CUSTOM_SIMPLE_AUTOMATION_TEST(FPLATEAUTest_ModelAdjustmentFilter_ApplyFilter_Toggles_Visibility, FPLATEAUAutomationTestBase,
                                        "PLATEAUTest.FPLATEAUTest.ModelAdjustmentFilter.ApplyFilter_Toggles_Visibility",
                                        EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPLATEAUTest_ModelAdjustmentFilter_ApplyFilter_Toggles_Visibility::RunTest(const FString& Parameters) {
    InitializeTest("ApplyFilter_Toggles_Visibility");
    if (!OpenNewMap()) AddError("Failed to OpenNewMap");

    const auto& Loader = GetInstancedCityLoader(*GetWorld());
    Loader->LoadAsync(true);

    ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Loader] {
        if (Loader->Phase != ECityModelLoadingPhase::Cancelling && Loader->Phase != ECityModelLoadingPhase::Finished)
            return false;

        TArray<AActor*> CityModelActors;
        UGameplayStatics::GetAllActorsOfClass(Loader->GetWorld(), APLATEAUInstancedCityModel::StaticClass(), CityModelActors);
        if (CityModelActors.Num() <= 0) AddError("CityModelActors.Num() <= 0");

        // フィルタリングによりBuildingパッケージを表示状態に変更
        for (auto* CityModelActor : CityModelActors) {
            APLATEAUInstancedCityModel* CityModel = Cast<APLATEAUInstancedCityModel>(CityModelActor);
            const auto CityModelPackages = CityModel->GetCityModelPackages();
            ApplyFilter(CityModel, CityModelPackages, true);

            TArray<USceneComponent*> GmlActors;
            CityModel->GetRootComponent()->GetChildrenComponents(false, GmlActors);
            if (GmlActors.Num() <= 0) AddError("GmlActors.Num() <= 0");

            const auto [MinLod, MaxLod] = CityModel->GetMinMaxLod(plateau::dataset::PredefinedCityModelPackage::Building);
            for (const auto& GmlActor : GmlActors) {
                TArray<USceneComponent*> LodActors;
                GmlActor->GetChildrenComponents(false, LodActors);
                
                for (const auto& LodActor : LodActors) {
                    auto LodNum = LodActor->GetName().RightChop(3);
                    if (FCString::Atoi(*LodNum) != MaxLod)
                        continue;
                    
                    TArray<USceneComponent*> PolygonMeshActors;
                    LodActor->GetChildrenComponents(false, PolygonMeshActors);
                    if (PolygonMeshActors.Num() <= 0) AddError("PolygonMeshActors.Num() <= 0");
                    if (!PolygonMeshActors[0]->IsVisible()) AddError("PolygonMeshActors[0]->IsVisible() == false");
                }
            }
        }

        // フィルタリングによりBuildingパッケージを非表示状態に変更
        for (auto* CityModelActor : CityModelActors) {
            APLATEAUInstancedCityModel* CityModel = Cast<APLATEAUInstancedCityModel>(CityModelActor);
            const auto CityModelPackages = CityModel->GetCityModelPackages();
            ApplyFilter(CityModel, CityModelPackages, false);

            TArray<USceneComponent*> GmlActors;
            CityModel->GetRootComponent()->GetChildrenComponents(false, GmlActors);
            const auto [MinLod, MaxLod] = CityModel->GetMinMaxLod(plateau::dataset::PredefinedCityModelPackage::Building);
            for (const auto& GmlActor : GmlActors) {
                TArray<USceneComponent*> LodActors;
                GmlActor->GetChildrenComponents(false, LodActors);

                for (const auto& LodActor : LodActors) {
                    auto LodNum = LodActor->GetName().RightChop(3);
                    if (FCString::Atoi(*LodNum) != MaxLod)
                        continue;
                    
                    TArray<USceneComponent*> PolygonMeshActors;
                    LodActor->GetChildrenComponents(false, PolygonMeshActors);
                    if (PolygonMeshActors[0]->IsVisible()) AddError("PolygonMeshActors[0]->IsVisible() == true");
                }
            }
        }
        
        return true;
    }));

    FinishTest();
    return true;
}
