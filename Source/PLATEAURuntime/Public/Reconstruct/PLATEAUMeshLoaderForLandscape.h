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

    /*
    Recommended Landscape Sizes
    Overall size(vertices)	Quads / section	Sections / Component	Landscape Component size	Total Landscape Components
    8129 x 8129	127	4 (2x2)	254x254	1024 (32x32)
    4033 x 4033	63	4 (2x2)	126x126	1024 (32x32)
    2017 x 2017	63	4 (2x2)	126x126	256 (16x16)
    1009 x 1009	63	4 (2x2)	126x126	64 (8x8)
    1009 x 1009	63	1	63x63	256 (16x16)
    505 x 505	63	4 (2x2)	126x126	16 (4x4)
    505 x 505	63	1	63x63	64 (8x8)
    253 x 253	63	4 (2x2)	126x126	4 (2x2)
    253 x 253	63	1	63x63	16 (4x4)
    127 x 127	63	4 (2x2)	126x126	1
    127 x 127	63	1	63x63	4 (2x2)
    */

    FPLATEAULandscapeParam() :
        TextureWidth(505), //
        TextureHeight(505),
        NumSubsections(2), //1 , 2
        SubsectionSizeQuads(63), //7, 15, 31, 63, 127, 255
        ComponentCountX(126),
        ComponentCountY(126),
        Offset(0,0) {}

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|BPLibraries|Landscape")
        FVector2D Offset;
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

    bool OverwriteTexture() override;


private:

};

