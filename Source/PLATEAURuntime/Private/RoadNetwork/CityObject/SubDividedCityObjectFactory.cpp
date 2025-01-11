// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "RoadNetwork/CityObject/SubDividedCityObjectFactory.h"
#include "BlueprintActionDatabase.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"
#include "UObject/FastReferenceCollector.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

TSharedPtr<FSubDividedCityObjectFactory::FConvertCityObjectResult>
FSubDividedCityObjectFactory::ConvertCityObjectsAsync(
    APLATEAUInstancedCityModel* Actor,
    const TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups,
    float Epsilon,
    bool UseContourMesh) {
    auto Result = MakeShared<FConvertCityObjectResult>();
    auto granularity = FPLATEAUModelReconstruct::GetConvertGranularityFromReconstructType(EPLATEAUMeshGranularity::PerAtomicFeatureObject);
    FPLATEAUModelReconstruct rec(Actor, granularity);
    auto model = rec.ConvertModelForReconstruct(CityObjectGroups);

   /* for (const auto& WeakCog : CityObjectGroups) 
    {
        if (auto* Cog = WeakCog.Get()) 
        {
            auto CityObjectInfo = FCityObjectInfo::Create(Cog, UseContourMesh);
            if (CityObjectInfo) {
                auto SubDividedCityObject = MakeShared<FSubDividedCityObject>(
                    Convert(TArray<TSharedPtr<FCityObjectInfo>>({ CityObjectInfo }),
                        MakeShared<FUnityMeshToDllSubMeshConverter>(),
                        true,
                        FAdderAndCoordinateSystemConverter(),
                        false),
                    FAttributeDataHelper());

                Result->ConvertedCityObjects->Add(SubDividedCityObject);
            }
        }
    }*/

    return Result;
}
