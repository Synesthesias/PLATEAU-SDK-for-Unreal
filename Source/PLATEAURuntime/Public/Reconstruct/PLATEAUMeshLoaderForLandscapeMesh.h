// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUMeshLoaderForLandscape.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForLandscapeMesh : public FPLATEAUMeshLoaderForLandscape {

public:
    FPLATEAUMeshLoaderForLandscapeMesh();
    FPLATEAUMeshLoaderForLandscapeMesh(const bool InbAutomationTest);

    void CreateMeshFromHeightMap(AActor& Actor, const int32 SizeX, const int32 SizeY, 
        const TVec3d Min, const TVec3d Max, 
        const TVec2f MinUV, const TVec2f MaxUV, 
        uint16_t* HeightRawData, 
        const FString NodeName);

protected:

    void LoadNodeRecursiveForHeightMap(
        const plateau::polygonMesh::Node& InNode,
        AActor& InActor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results);
    void LoadNodeForHeightMap(
        const plateau::polygonMesh::Node& Node,
        AActor& Actor, FPLATEAULandscapeParam Param, TArray<HeightmapCreationResult> &Results);
    HeightmapCreationResult CreateHeightMapFromMesh(
        const plateau::polygonMesh::Mesh& InMesh,
        const FString NodeName,
        AActor& Actor, FPLATEAULandscapeParam Param);

    UStaticMeshComponent* GetStaticMeshComponentForCondition(AActor& Actor, EName Name, const std::string& InNodeName,
        const plateau::polygonMesh::Mesh& InMesh, const FLoadInputData& LoadInputData,
        const std::shared_ptr <const citygml::CityModel> CityModel) override;
    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FString NodeName) override;

    bool OverwriteTexture() override;
    bool InvertMeshNormal() override;
    bool MergeTriangles() override;
    void ModifyMeshDescription(FMeshDescription& MeshDescription) override;

    TArray<USceneComponent*> FindComponentsByName(AActor* ModelActor, FString Name);
    UPLATEAUCityObjectGroup* GetOriginalComponent(AActor& Actor, FString Name);

private:
    UMaterialInstanceDynamic* ReplaceMaterial;

};

