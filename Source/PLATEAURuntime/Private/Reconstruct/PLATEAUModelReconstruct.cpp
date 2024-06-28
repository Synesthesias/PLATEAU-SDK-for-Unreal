// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUModelReconstruct.h"
#include <plateau/granularity_convert/granularity_converter.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

using namespace plateau::granularityConvert;

FPLATEAUModelReconstruct::FPLATEAUModelReconstruct() {}

FPLATEAUModelReconstruct::FPLATEAUModelReconstruct(APLATEAUInstancedCityModel* Actor, const plateau::polygonMesh::MeshGranularity Granularity) {
    CityModelActor = Actor;
    //MeshGranularity = static_cast<plateau::polygonMesh::MeshGranularity>(ReconstructType);
    MeshGranularity = Granularity;
    bDivideGrid = false;
}

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

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelReconstruct::FilterComponentsByMeshGranularity(TArray<UPLATEAUCityObjectGroup*> TargetComponents, const plateau::polygonMesh::MeshGranularity Granularity){
    TArray<UPLATEAUCityObjectGroup*> Components = TargetComponents.FilterByPredicate([Granularity](UPLATEAUCityObjectGroup* Component) {
        return Component->GetMeshGranularity() == Granularity;
        });
    return Components;
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
    return ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}

TArray<USceneComponent*> FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(FPLATEAUMeshLoaderForReconstruct& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model) {
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(CityModelActor->GetRootComponent(), Model->getRootNodeAt(i), MeshGranularity, CityObjMap, *CityModelActor);
    }
    return MeshLoader.GetLastCreatedComponents();
}

plateau::polygonMesh::MeshGranularity FPLATEAUModelReconstruct::GetMeshGranularityFromReconstructType(const EPLATEAUMeshGranularity ReconstructType) {
    switch (ReconstructType) {
    case EPLATEAUMeshGranularity::PerAtomicFeatureObject:
        return plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
    case EPLATEAUMeshGranularity::PerPrimaryFeatureObject:
        return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
    case EPLATEAUMeshGranularity::PerCityModelArea:
        return plateau::polygonMesh::MeshGranularity::PerCityModelArea;
    case EPLATEAUMeshGranularity::PerMaterialInPrimary:
        return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
    default:
        return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
    }
}