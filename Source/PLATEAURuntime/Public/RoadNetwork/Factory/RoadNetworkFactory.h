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


UCLASS(BlueprintType)
class PLATEAURUNTIME_API URoadNetworkFactory : public UObject 
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

    RnRef_t<RnModel::CalibrateIntersectionBorderOption> CalibrateIntersectionOption;
    FRGraphFactory GraphFactory;

    struct FCreateRnModelRequest
    {
        APLATEAUInstancedCityModel* Actor;
        TWeakObjectPtr<USceneComponent> Transform;
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
        //PLATEAURnStructureModel* OriginalMesh;
    };

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
    void CreateRnModel(APLATEAUInstancedCityModel* Actor, AActor* DestActor);

    RnRef_t<RnModel> CreateRoadNetwork(APLATEAUInstancedCityModel* Actor, AActor* DestActor, TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups);

    RnRef_t<RnModel> CreateRoadNetwork(RGraphRef_t<URGraph> Graph);

private:
    // 最小地物に分解する
    void CreateSubDividedCityObjects(APLATEAUInstancedCityModel* Actor
        , AActor* DestActor
        , USceneComponent* Root
        , TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups
        , TArray<FSubDividedCityObject>& OutSubDividedCityObjects);

    void CreateRGraph(APLATEAUInstancedCityModel* Actor
        , AActor* DestActor
        , USceneComponent* Root
        , TArray<UPLATEAUCityObjectGroup*>& CityObjectGroups
        , TArray<FSubDividedCityObject>& SubDividedCityObjects
        , RGraphRef_t<URGraph>& OutGraph);
};
