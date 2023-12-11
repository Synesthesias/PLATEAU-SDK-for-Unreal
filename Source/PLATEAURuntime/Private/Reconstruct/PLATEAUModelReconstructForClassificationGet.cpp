// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUModelReconstructForClassificationGet.h"
#include "Tasks/Task.h"
#include "Misc/DefaultValueHelper.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "CityGML/PLATEAUCityGmlProxy.h"
#include <PLATEAUMeshExporter.h>
#include <PLATEAUMeshLoader.h>
#include <PLATEAUExportSettings.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassificationGet.h>

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelReconstructForClassificationGet::GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents) {
    TSet<UPLATEAUCityObjectGroup*> UniqueComponents;
    for (auto comp : TargetComponents) {
        if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && comp->IsVisible()) {
            TArray<USceneComponent*> children;
            comp->GetChildrenComponents(true, children);
            for (auto child : children) {
                if (child->IsA(UPLATEAUCityObjectGroup::StaticClass()) && child->IsVisible()) {
                    auto childCityObj = StaticCast<UPLATEAUCityObjectGroup*>(child);
                    UniqueComponents.Add(childCityObj);
                }
            }
        }
        if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible())
            UniqueComponents.Add(StaticCast<UPLATEAUCityObjectGroup*>(comp));
    }
    return UniqueComponents.Array();
}

TArray<USceneComponent*> FPLATEAUModelReconstructForClassificationGet::ReconstructFromConvertedModelForClassificationGet(std::shared_ptr<plateau::polygonMesh::Model> Model, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials) {

    FPLATEAUMeshLoaderForClassificationGet MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationMaterials(ClassificationMaterials);  
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
