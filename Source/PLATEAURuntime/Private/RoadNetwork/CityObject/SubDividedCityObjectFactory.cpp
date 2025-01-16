// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "RoadNetwork/CityObject/SubDividedCityObjectFactory.h"
#include "BlueprintActionDatabase.h"
#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"
#include "UObject/FastReferenceCollector.h"
#include <plateau/granularity_convert/granularity_converter.h>

#include "CityGML/PLATEAUCityObject.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

namespace
{
    class PLATEAURUNTIME_API TmpLoader : public FPLATEAUModelReconstruct
    {
    public:
        TmpLoader(APLATEAUInstancedCityModel* Actor, const ConvertGranularity Granularity)
            : FPLATEAUModelReconstruct(Actor, Granularity) {
        }

        TMap<FString, FPLATEAUCityObject>& GetCityObjMap()
        {
            return CityObjMap;
        }
    };
}
TSharedPtr<FSubDividedCityObjectFactory::FConvertCityObjectResult>
FSubDividedCityObjectFactory::ConvertCityObjectsAsync(
    APLATEAUInstancedCityModel* Actor,
    const TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups,
    float Epsilon,
    bool UseContourMesh)
{
    auto Result = MakeShared<FConvertCityObjectResult>();
    auto Granularity = FPLATEAUModelReconstruct::GetConvertGranularityFromReconstructType(EPLATEAUMeshGranularity::PerAtomicFeatureObject);
    ::TmpLoader Loader(Actor, Granularity);
    auto model = Loader.ConvertModelForReconstruct(CityObjectGroups);


    for(auto i = 0; i < model->getRootNodeCount(); ++i)
    {
        auto& Node = model->getRootNodeAt(i);
        auto SO = MakeShared<FSubDividedCityObject>(Node, Loader.GetCityObjMap(), ERRoadTypeMask::Empty);
        Result->ConvertedCityObjects.Add(SO);
    }

    return Result;
}
