// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
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

    TArray<HeightmapCreationResult> CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param);

protected:


};

