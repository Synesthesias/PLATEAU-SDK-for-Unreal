// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelAlignLand : public FPLATEAUModelReconstruct {

public:
    FPLATEAUModelAlignLand();
    FPLATEAUModelAlignLand(APLATEAUInstancedCityModel* Actor);

    /**
     * @brief 元のMeshGranularityを記憶します
     * @param TargetCityObjects UPLATEAUCityObjectGroupのリスト
     * @return Key: GmlID, Value: MeshGranularity の Map
     */
    TMap<FString, plateau::polygonMesh::MeshGranularity> CreateMeshGranularityMap(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects);


    void SetHeightData(const std::vector<uint16_t> HeightData, const TVec3d Min, const TVec3d Max, const FString NodeName, FPLATEAULandscapeParam Param);


protected:

    TMap<FString, plateau::polygonMesh::MeshGranularity> MeshGranularityMap;
};

