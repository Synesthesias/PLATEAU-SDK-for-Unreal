// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include <Landscape.h>
#include "Reconstruct/PLATEAUModelReconstruct.h"

struct FPLATEAULandscapeParam;

class PLATEAURUNTIME_API FPLATEAUModelLandscape : public FPLATEAUModelReconstruct {

public:
    FPLATEAUModelLandscape();
    FPLATEAUModelLandscape(APLATEAUInstancedCityModel* Actor);

    /**
     * @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してtypeがTINReliefの場合のみリストに追加します
     */
    TArray<UPLATEAUCityObjectGroup*> GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents) override;

    TArray<HeightmapCreationResult> CreateHeightMap(std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param);

    ALandscape* CreateLandScape(UWorld* World, const int32 NumSubsections, const int32 SubsectionSizeQuads, const int32 ComponentCountX, const int32 ComponentCountY, const int32 SizeX, const int32 SizeY,
        const TVec3d Min, const TVec3d Max, const TVec2f MinUV, const TVec2f MaxUV, const FString TexturePath, TArray<uint16> HeightData, const FString ActorName);

    void CreateLandScapeReference(ALandscape* Landscape, AActor* Actor, const FString ActorName);

protected:


};

