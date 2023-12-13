// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForReconstruct : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForReconstruct();
    FPLATEAUMeshLoaderForReconstruct(const bool InbAutomationTest);

    void ReloadComponentFromNode(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        TMap<FString, FPLATEAUCityObject> cityObjMap,
        AActor& InActor);

protected:

    void ReloadNodeRecursive(
        USceneComponent* InParentComponent,
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& InActor);
    USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor);

    UMaterialInstanceDynamic* ReplaceMaterialForTexture(const FString TexturePath) override;

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName, 
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData, 
        const std::shared_ptr <const citygml::CityModel> CityModel) override;

    bool InvertMeshNormal() override;

    //分割・結合時に属性情報を保持　
    TMap<FString, FPLATEAUCityObject> CityObjMap;

private:
};
