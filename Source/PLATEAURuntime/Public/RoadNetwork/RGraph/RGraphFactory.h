#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <memory>

#include "RGraph.h"

class FSubDividedCityObject;
class UPLATEAUCityObjectGroup;
class RnModel;
class URGraph;

class FRGraphFactory {
public:
    bool bReductionOnCreate = true;
    float MergeCellSize = 0.5f;
    int32 MergeCellLength = 4;
    float RemoveMidPointTolerance = 0.3f;
    float Lod1HeightTolerance = 1.5f;
    bool bUseCityObjectOutline = true;

    RGraphRef_t<URGraph> CreateGraph(const TArray<FSubDividedCityObject>& CityObjects);
};
