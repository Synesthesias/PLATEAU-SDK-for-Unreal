// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUModelReconstruct.h"
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

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

/**
* @brief UPLATEAUCityObjectGroupのリストからUPLATEAUCityObjectを取り出し、GmlIDをキーとしたMapを生成
* @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
* @return Key: GmlID, Value: UPLATEAUCityObject の Map
*/
TMap<FString, FPLATEAUCityObject> FPLATEAUModelReconstruct::CreateMapFromCityObjectGroups(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjectGroups) {
    TMap<FString, FPLATEAUCityObject> cityObjMap;
    for (auto comp : TargetCityObjectGroups) {

        if (comp->SerializedCityObjects.IsEmpty())
            continue;

        for (auto cityObj : comp->GetAllRootCityObjects()) {
            if (!comp->OutsideParent.IsEmpty() && !cityObjMap.Contains(comp->OutsideParent)) {
                // 親を探す
                TArray<USceneComponent*> Parents;
                comp->GetParentComponents(Parents);
                for (const auto& Parent : Parents) {
                    if (Parent->GetName().Contains(comp->OutsideParent)) {
                        for (auto pObj : Cast<UPLATEAUCityObjectGroup>(Parent)->GetAllRootCityObjects()) {
                            cityObjMap.Add(pObj.GmlID, pObj);
                        }
                        break;
                    }
                }
            }

            cityObjMap.Add(cityObj.GmlID, cityObj);
            for (auto child : cityObj.Children) {
                cityObjMap.Add(child.GmlID, child);
            }
        }
    }
    return cityObjMap;
}

/**
* @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してリストに追加します
*/
TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelReconstruct::GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents) {
    TSet<UPLATEAUCityObjectGroup*> UniqueComponents;
    for (auto comp : TargetComponents) {
        if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && StaticCast<UStaticMeshComponent*>(comp)->GetStaticMesh() == nullptr && comp->IsVisible()) {
            TArray<USceneComponent*> children;
            comp->GetChildrenComponents(true, children);
            for (auto child : children) {
                if (child->IsA(UPLATEAUCityObjectGroup::StaticClass()) && child->IsVisible()) {
                    auto childCityObj = StaticCast<UPLATEAUCityObjectGroup*>(child);
                    if (childCityObj->GetStaticMesh() != nullptr) {
                        UniqueComponents.Add(childCityObj);
                    }
                }
            }
        }
        if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible())
            UniqueComponents.Add(StaticCast<UPLATEAUCityObjectGroup*>(comp));
    }
    return UniqueComponents.Array();
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstruct::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    GranularityConvertOption ConvOption(MeshGranularity, bDivideGrid ? 1 : 0);

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;

    FPLATEAUMeshExporter MeshExporter;
    GranularityConverter Converter;

    //属性情報を覚えておきます。
    CityObjMap = CreateMapFromCityObjectGroups(TargetCityObjects);

    check(CityModelActor != nullptr);

    std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    std::shared_ptr<plateau::polygonMesh::Model> converted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*smodel, ConvOption));

    return converted;
}

TArray<USceneComponent*> FPLATEAUModelReconstruct::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {
    FPLATEAUMeshLoaderForReconstruct MeshLoader(false);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}

TArray<USceneComponent*> FPLATEAUModelReconstruct::ReconstructFromConvertedModel(FPLATEAUMeshLoaderForReconstruct& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model) {
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(CityModelActor->GetRootComponent(), Model->getRootNodeAt(i), MeshGranularity, CityObjMap, *CityModelActor);
    }
    return MeshLoader.GetLastCreatedComponents();
}