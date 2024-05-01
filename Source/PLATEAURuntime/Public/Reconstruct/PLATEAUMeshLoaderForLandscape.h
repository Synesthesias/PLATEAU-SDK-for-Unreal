// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUMeshLoaderForLandscape.generated.h"

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAULandscapeParam {
    GENERATED_BODY()

    FPLATEAULandscapeParam() :
        TextureWidth(513), //513, 1024
        TextureHeight(513), 
        NumSubsections(1), //1 , 2
        SubsectionSizeQuads(127), //7, 15, 31, 63, 127, 255
        ComponentCountX(2), 
        ComponentCountY(2) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 TextureWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 TextureHeight;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 NumSubsections; //1 , 2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 SubsectionSizeQuads;  //7, 15, 31, 63, 127, 255
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 ComponentCountX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        int32 ComponentCountY;
};

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForLandscape : public FPLATEAUMeshLoader {

public:
    FPLATEAUMeshLoaderForLandscape();
    FPLATEAUMeshLoaderForLandscape(const bool InbAutomationTest);

    void CreateHeightMap(
        AActor* ModelActor,
        const std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param);

    void CreateLandScape(UWorld* World);
    void CreateLandScape(UWorld* World, const int32 NumSubsections, const int32 SubsectionSizeQuads, const int32 ComponentCountX, const int32 ComponentCountY, const int32 SizeX, const int32 SizeY,
        const TVec3d Min, const TVec3d Max, const TVec2f MinUV, const TVec2f MaxUV, const FString TexturePath, TArray<uint16> HeightData, const FString ActorName);

protected:

    void LoadNodeRecursiveForHeightMap(
        const plateau::polygonMesh::Node& InNode,
        AActor& InActor, FPLATEAULandscapeParam Param);
    void LoadNodeForHeightMap(
        const plateau::polygonMesh::Node& Node,
        AActor& Actor, FPLATEAULandscapeParam Param);
    void CreateHeightMapFromMesh(
        const plateau::polygonMesh::Mesh& InMesh,
        const FString NodeName,
        AActor& Actor, FPLATEAULandscapeParam Param);


private:

};

