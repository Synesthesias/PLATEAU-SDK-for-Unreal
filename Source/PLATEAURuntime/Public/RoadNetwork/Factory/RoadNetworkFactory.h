#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "../RGraph/RGraphDef.h"
#include <memory>
#include <functional>

#include "PLATEAUInstancedCityModel.h"
#include "RoadNetwork/RGraph/RGraphFactory.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetworkFactory.generated.h"

class UPLATEAUCityObjectGroup;
class RnModel;
class RnRoad;
class RnIntersection;
class RnSideWalk;
class RnWay;
class RnLineString;
class RnLane;
class RnPoint;


USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FRoadNetworkFactory
{
    GENERATED_BODY()
public:
    static const FString FactoryVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    float RoadSize = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    float TerminateAllowEdgeAngle = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    float TerminateSkipAngle = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    float Lod1SideWalkSize = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    float Lod1SideWalkThresholdRoadWidth = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bAddSideWalk = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bCheckMedian = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bIgnoreHighway = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bAddTrafficSignalLights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bSaveTmpData = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bUseContourMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bMergeRoadGroup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bCalibrateIntersection = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    bool bSeparateContinuousBorder = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    FRGraphFactory GraphFactory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PLATEAU")
    FRnModelCalibrateIntersectionBorderOption CalibrateIntersectionOption;

};

struct FRoadNetworkFactoryEx
{

    struct FCreateRnModelRequest {
        APLATEAUInstancedCityModel* Actor;
        TWeakObjectPtr<USceneComponent> Transform;
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
        //PLATEAURnStructureModel* OriginalMesh;
    };

    static void CreateRnModel(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor);

    static RnRef_t<RnModel> CreateRoadNetwork(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor, AActor* DestActor, TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups);

    static  RnRef_t<RnModel> CreateRoadNetwork(const FRoadNetworkFactory& Self, RGraphRef_t<URGraph> Graph);
private:
    // 最小地物に分解する
    static void CreateSubDividedCityObjects(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor
        , AActor* DestActor
        , USceneComponent* Root
        , TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups
        , TArray<FSubDividedCityObject>& OutSubDividedCityObjects);

    static void CreateRGraph(const FRoadNetworkFactory& Self, APLATEAUInstancedCityModel* Actor
        , AActor* DestActor
        , USceneComponent* Root
        , TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups
        , TArray<FSubDividedCityObject>& SubDividedCityObjects
        , RGraphRef_t<URGraph>& OutGraph);
};