// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Reconstruct/PLATEAUMeshLoaderCloneComponent.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"
#include "PLATEAUInstancedCityModel.h"

FPLATEAUMeshLoaderCloneComponent::FPLATEAUMeshLoaderCloneComponent() {}

FPLATEAUMeshLoaderCloneComponent::FPLATEAUMeshLoaderCloneComponent(const bool InbAutomationTest){
    bAutomationTest = InbAutomationTest;
}

TMap<FString, UPLATEAUCityObjectGroup*> FPLATEAUMeshLoaderCloneComponent::CreateComponentsMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    TMap<FString, UPLATEAUCityObjectGroup*> Map;
    for (auto Comp : TargetCityObjects) {
        Map.Add(APLATEAUInstancedCityModel::GetOriginalComponentName(Comp), Comp);
    }
    return Map;
}

UPLATEAUCityObjectGroup* FPLATEAUMeshLoaderCloneComponent::GetOriginalComponent(FString Name) {
    auto Ptr = ComponentsMap.Find(Name);
    if (Ptr) {
        const auto& OriginalComponent = *Ptr;
        return OriginalComponent;
    }
    return nullptr;
}

void FPLATEAUMeshLoaderCloneComponent::ReloadComponentFromNode(
    const plateau::polygonMesh::Node& InNode,
    TMap<FString, UPLATEAUCityObjectGroup*> Components,
    AActor& InActor) {

    ComponentsMap = Components;
    FPLATEAUMeshLoaderForReconstruct::ReloadComponentFromNode(nullptr, InNode, plateau::granularityConvert::ConvertGranularity::PerPrimaryFeatureObject, TMap<FString, FPLATEAUCityObject>(), InActor);
}

UStaticMeshComponent* FPLATEAUMeshLoaderCloneComponent::GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
    const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
    const std::shared_ptr <const citygml::CityModel> CityModel) {

    const FString NodeName = UTF8_TO_TCHAR(InNodeName.c_str());
    const auto& PLATEAUCityObjectGroup = NewObject<UPLATEAUCityObjectGroup>(&Actor, NAME_None);

    // Originalコンポーネントの属性をそのまま利用
    const auto& OriginalComponent = GetOriginalComponent(NodeName);
    if (OriginalComponent) {
        PLATEAUCityObjectGroup->SerializedCityObjects = OriginalComponent->SerializedCityObjects;
        PLATEAUCityObjectGroup->OutsideChildren = OriginalComponent->OutsideChildren;
        PLATEAUCityObjectGroup->OutsideParent = OriginalComponent->OutsideParent;
        PLATEAUCityObjectGroup->MeshGranularityIntValue = OriginalComponent->MeshGranularityIntValue;
    }
    return PLATEAUCityObjectGroup;
}

UMaterialInstanceDynamic* FPLATEAUMeshLoaderCloneComponent::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) {

    // Originalコンポーネントのマテリアルをそのまま利用
    const auto& OriginalComponent = GetOriginalComponent(NodeName);
    if (OriginalComponent)
        return (UMaterialInstanceDynamic*)OriginalComponent->GetMaterial(0);

    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture, NodeName);
}

/**
 * @brief Meshとコンポーネントが存在する場合、Originalコンポーネントと同一階層にCloneを配置します。それ以外は処理しません。
 * ParentComponent, MeshGranularityパラメータは無視してコンポーネントの値を利用
 */
USceneComponent* FPLATEAUMeshLoaderCloneComponent::ReloadNode(USceneComponent* ParentComponent,
    const plateau::polygonMesh::Node& Node,
    ConvertGranularity Granularity,
    AActor& Actor) {

    if (Node.getMesh() != nullptr && Node.getMesh()->getVertices().size() > 0) {

        const FString CompName = FString(UTF8_TO_TCHAR(Node.getName().c_str()));
        const auto& OriginalComponent = GetOriginalComponent(CompName);

        if (OriginalComponent) {

            const auto& OriginalParentComponent = OriginalComponent->GetAttachParent();
            const auto& OriginalGranularity = StaticCast<plateau::polygonMesh::MeshGranularity>(OriginalComponent->MeshGranularityIntValue);

            plateau::polygonMesh::MeshExtractOptions MeshExtractOptions{};
            MeshExtractOptions.mesh_granularity = OriginalGranularity;
            FLoadInputData LoadInputData
            {
                MeshExtractOptions,
                std::vector<plateau::geometry::Extent>{},
                FString(),
                false,
                nullptr
            };
            return CreateStaticMeshComponent(Actor, *OriginalParentComponent, *Node.getMesh(), LoadInputData, nullptr,
                Node.getName());
        }
    }
    return nullptr;
}

