#pragma once

#include "CoreMinimal.h"
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory")
    bool bUseCityObjectOutline = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    float MergeCellSize = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    int32 MergeCellLength = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    float RemoveMidPointTolerance = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    float Lod1HeightTolerance = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptAdjustSmallLodHeight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptEdgeReduction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptVertexReduction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptRemoveIsolatedEdgeFromFace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptInsertVertexInNearEdge = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU|Factory|Optimize")
    bool bOptSeparateFaces = true;
};

struct FRGraphFactoryEx
{
    static RGraphRef_t<URGraph> CreateGraph(const FRGraphFactory& Factory, const TArray<FSubDividedCityObject>& CityObjects);
};
