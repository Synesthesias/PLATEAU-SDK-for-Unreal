// Copyright 2023 Ministry of Land, Infrastructure and Transport


#include "Reconstruct/PLATEAUMeshLoaderForAlignLand.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUCityObjectGroup.h"

FPLATEAUMeshLoaderForAlignLand::FPLATEAUMeshLoaderForAlignLand() {}

FPLATEAUMeshLoaderForAlignLand::FPLATEAUMeshLoaderForAlignLand(const bool InbAutomationTest){
    bAutomationTest = InbAutomationTest;
}

UPLATEAUCityObjectGroup* FPLATEAUMeshLoaderForAlignLand::GetOriginalComponent(FString Name) {
    auto Ptr = ComponentsMap.Find(Name);
    if (Ptr) {
        const auto& OriginalComponent = *Ptr;
        return OriginalComponent;
    }
    return nullptr;
}

void FPLATEAUMeshLoaderForAlignLand::ReloadComponentFromNode(
    const plateau::polygonMesh::Node& InNode,
    plateau::polygonMesh::MeshGranularity Granularity,
    TMap<FString, UPLATEAUCityObjectGroup*> Components,
    AActor& InActor) {

    ComponentsMap = Components;
    FPLATEAUMeshLoaderForReconstruct::ReloadComponentFromNode(nullptr, InNode, Granularity, TMap<FString, FPLATEAUCityObject>(), InActor);
}

UStaticMeshComponent* FPLATEAUMeshLoaderForAlignLand::GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
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

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForAlignLand::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) {

    // Originalコンポーネントのマテリアルをそのまま利用
    const auto& OriginalComponent = GetOriginalComponent(NodeName);
    if (OriginalComponent)
        return (UMaterialInstanceDynamic*)OriginalComponent->GetMaterial(0);

    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture, NodeName);
}

/**
 * @brief Originalコンポーネントと同一階層に配置します。それ以外は処理しません。
 */
USceneComponent* FPLATEAUMeshLoaderForAlignLand::ReloadNode(USceneComponent* ParentComponent,
    const plateau::polygonMesh::Node& Node,
    plateau::polygonMesh::MeshGranularity Granularity,
    AActor& Actor) {

    UE_LOG(LogTemp, Error, TEXT("FPLATEAUMeshLoaderForAlignLand :: ReloadNode "));

    if (Node.getMesh() != nullptr && Node.getMesh()->getVertices().size() > 0) {

        const FString CompName = FString(UTF8_TO_TCHAR(Node.getName().c_str()));
        const auto& OriginalComponent = GetOriginalComponent(CompName);
        if (OriginalComponent) {
            plateau::polygonMesh::MeshExtractOptions MeshExtractOptions{};
            MeshExtractOptions.mesh_granularity = Granularity;
            FLoadInputData LoadInputData
            {
                MeshExtractOptions,
                std::vector<plateau::geometry::Extent>{},
                FString(),
                false,
                nullptr
            };

            ParentComponent = OriginalComponent->GetAttachParent();
            return CreateStaticMeshComponent(Actor, *ParentComponent, *Node.getMesh(), LoadInputData, nullptr,
                Node.getName());
        }
    }
    return nullptr;
}

