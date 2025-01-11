#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <memory>

class FSubDividedCityObject;
class UPLATEAUCityObjectGroup;
class RnModel;
class FRGraph;

class FRGraphFactory {
public:
    bool bReductionOnCreate = true;
    float MergeCellSize = 0.5f;
    int32 MergeCellLength = 4;
    float RemoveMidPointTolerance = 0.3f;
    float Lod1HeightTolerance = 1.5f;
    bool bUseCityObjectOutline = true;

    RnRef_t<FRGraph> CreateGraph(const TArray<TSharedPtr<FSubDividedCityObject>>& CityObjects);
};
