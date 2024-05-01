// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"

#include "PLATEAUGeometry.h"
#include "PLATEAUCityModelLoader.h"

class PLATEAURUNTIME_API FPLATEAUHeightMapCreator : public FPLATEAUMeshLoader {

public:
    FPLATEAUHeightMapCreator();
    FPLATEAUHeightMapCreator(const bool InbAutomationTest);

    //void CalculateExtent(plateau::polygonMesh::MeshExtractOptions options, std::vector<plateau::geometry::Extent> Extents);

    void CreateHeightMap(
        AActor* ModelActor,
        const std::shared_ptr<plateau::polygonMesh::Model> Model,
        const FLoadInputData& LoadInputData,
        const std::shared_ptr<const citygml::CityModel> CityModel);

    void CreateHeightMap(
        AActor* ModelActor,
        const std::shared_ptr<plateau::polygonMesh::Model> Model);



    void CreateLandScape(UWorld* World);
    void CreateLandScape(UWorld* World, const int32 NumSubsections, const int32 SubsectionSizeQuads, const int32 ComponentCountX, const int32 ComponentCountY, const int32 SizeX, const int32 SizeY,
        const TVec3d Min, const TVec3d Max, const TVec2f MinUV, const TVec2f MaxUV, const FString TexturePath, TArray<uint16> HeightData, const FString ActorName);

protected:

    int32 TextureWidth = 513;
    int32 TextureHeight = 513;
    //int32 TextureWidth = 1024;
    //nt32 TextureHeight = 1024;

    void LoadNodeRecursiveForHeightMap(
        const plateau::polygonMesh::Node& InNode,
        AActor& InActor);
    void LoadNodeForHeightMap(
        const plateau::polygonMesh::Node& Node,
        AActor& Actor);
    void CreateHeightMapFromMesh(
        const plateau::polygonMesh::Mesh& InMesh,
        const FString NodeName,
        AActor& Actor);


private:

};

