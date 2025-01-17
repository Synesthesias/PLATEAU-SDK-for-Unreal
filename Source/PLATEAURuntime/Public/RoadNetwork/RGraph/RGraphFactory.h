#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <memory>

#include "RGraph.h"
#include "RGraphFactory.generated.h"
class FSubDividedCityObject;
class UPLATEAUCityObjectGroup;
class URnModel;
class URGraph;

USTRUCT(BlueprintType)
struct FRGraphFactory
{
    GENERATED_BODY();
public:
    UPROPERTY(EditAnywhere, Category="PLATEAU|Factory")
    bool bReductionOnCreate = true;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Factory")
    float MergeCellSize = 0.5f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Factory")
    int32 MergeCellLength = 4;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Factory")
    float RemoveMidPointTolerance = 0.3f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Factory")
    float Lod1HeightTolerance = 1.5f;

    UPROPERTY(EditAnywhere, Category = "PLATEAU|Factory")
    bool bUseCityObjectOutline = true;
};

struct FRGraphFactoryEx
{
    static RGraphRef_t<URGraph> CreateGraph(const FRGraphFactory& Factory, const TArray<FSubDividedCityObject>& CityObjects);
};
