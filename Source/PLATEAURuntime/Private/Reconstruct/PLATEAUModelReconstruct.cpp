// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUModelReconstruct.h"
#include <plateau/granularity_convert/granularity_converter.h>
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

using namespace plateau::granularityConvert;

FPLATEAUModelReconstruct::FPLATEAUModelReconstruct() {}

FPLATEAUModelReconstruct::FPLATEAUModelReconstruct(APLATEAUInstancedCityModel* Actor, const ConvertGranularity Granularity) {
    CityModelActor = Actor;
    ConvGranularity = Granularity;
    bDivideGrid = false;
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

TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelReconstruct::FilterComponentsByConvertGranularity(TArray<UPLATEAUCityObjectGroup*> TargetComponents, const ConvertGranularity Granularity) {
    TArray<UPLATEAUCityObjectGroup*> Components = TargetComponents.FilterByPredicate([Granularity](UPLATEAUCityObjectGroup* Component) {
        return Component->GetConvertGranularity() == Granularity;
        });
    return Components;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstruct::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    return ConvertModelWithGranularity(TargetCityObjects, ConvGranularity);
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstruct::ConvertModelWithGranularity(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const ConvertGranularity Granularity) {

    auto OriginalGranularity = ConvGranularity;
    ConvGranularity = Granularity;

    GranularityConvertOption ConvOption(Granularity, bDivideGrid ? 1 : 0);

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;

    FPLATEAUMeshExporter MeshExporter;
    GranularityConverter Converter;

    //属性情報を覚えておきます。
    CityObjMap = FPLATEAUMeshLoaderForReconstruct::CreateMapFromCityObjectGroups(TargetCityObjects);

    check(CityModelActor != nullptr);

    std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    std::shared_ptr<plateau::polygonMesh::Model> converted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*smodel, ConvOption));

    ConvGranularity = OriginalGranularity;

    return converted;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstruct::CreateModel(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;

    FPLATEAUMeshExporter MeshExporter;
    check(CityModelActor != nullptr);
    std::shared_ptr<plateau::polygonMesh::Model> smodel = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);
    return smodel;
}

TArray<USceneComponent*> FPLATEAUModelReconstruct::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {
    FPLATEAUMeshLoaderForReconstruct MeshLoader(false);
    return ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}

TArray<USceneComponent*> FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(FPLATEAUMeshLoaderForReconstruct& MeshLoader, std::shared_ptr<plateau::polygonMesh::Model> Model) {
    for (int i = 0; i < Model->getRootNodeCount(); i++) {
        MeshLoader.ReloadComponentFromNode(CityModelActor->GetRootComponent(), Model->getRootNodeAt(i), ConvGranularity, CityObjMap, *CityModelActor);
    }
    return MeshLoader.GetLastCreatedComponents();
}

ConvertGranularity FPLATEAUModelReconstruct::GetConvertGranularityFromReconstructType(const EPLATEAUMeshGranularity ReconstructType) {
    switch (ReconstructType) {
    case EPLATEAUMeshGranularity::PerAtomicFeatureObject:
        return ConvertGranularity::PerAtomicFeatureObject;
    case EPLATEAUMeshGranularity::PerPrimaryFeatureObject:
        return ConvertGranularity::PerPrimaryFeatureObject;
    case EPLATEAUMeshGranularity::PerCityModelArea:
        return ConvertGranularity::PerCityModelArea;
    case EPLATEAUMeshGranularity::PerMaterialInPrimary:
        return ConvertGranularity::MaterialInPrimary;
    default:
        return ConvertGranularity::PerPrimaryFeatureObject;
    }
}

void FPLATEAUModelReconstruct::SetMeshGranularity(const EPLATEAUMeshGranularity Granularity) {
    ConvGranularity = FPLATEAUModelReconstruct::GetConvertGranularityFromReconstructType(Granularity);
    bChangeGranularity = Granularity != EPLATEAUMeshGranularity::DoNotChange;
}

void FPLATEAUModelReconstruct::GetChildrenGmlIds(const FPLATEAUCityObject CityObj, TSet<FString>& IdList) {
    for (auto child : CityObj.Children) {
        IdList.Add(child.GmlID);
        GetChildrenGmlIds(child, IdList);
    }
}