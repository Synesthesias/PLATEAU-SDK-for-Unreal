// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUMeshLoaderForReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForAlignLand : public FPLATEAUMeshLoaderForReconstruct {

public:
    FPLATEAUMeshLoaderForAlignLand();
    FPLATEAUMeshLoaderForAlignLand(const bool InbAutomationTest);

    void ReloadComponentFromNode(
        const plateau::polygonMesh::Node& InNode,
        plateau::polygonMesh::MeshGranularity Granularity,
        TMap<FString, UPLATEAUCityObjectGroup*> Components,
        AActor& InActor);

    USceneComponent* ReloadNode(
        USceneComponent* ParentComponent,
        const plateau::polygonMesh::Node& Node,
        plateau::polygonMesh::MeshGranularity Granularity,
        AActor& Actor) override;

protected:

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
        const std::shared_ptr <const citygml::CityModel> CityModel) override;

    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) override;

    UPLATEAUCityObjectGroup* GetOriginalComponent(FString Name);

private:

    //元のコンポーネント情報を保持　
    TMap<FString, UPLATEAUCityObjectGroup*> ComponentsMap;

};

