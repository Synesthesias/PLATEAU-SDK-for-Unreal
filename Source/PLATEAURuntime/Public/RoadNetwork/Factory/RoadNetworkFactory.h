#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "../RGraph/RGraphDef.h"
#include <memory>
#include <functional>

#include "PLATEAUInstancedCityModel.h"
#include "RoadNetwork/RGraph/RGraphFactory.h"
#include "RoadNetwork/Structure/RnModel.h"

class UPLATEAUCityObjectGroup;
class RnModel;
class RnRoad;
class RnIntersection;
class RnSideWalk;
class RnWay;
class RnLineString;
class RnLane;
class RnPoint;

class FRoadNetworkFactory {
public:
    static const FString FactoryVersion;

    float RoadSize = 3.0f;
    float TerminateAllowEdgeAngle = 20.0f;
    float TerminateSkipAngle = 5.0f;
    float Lod1SideWalkSize = 3.0f;
    float Lod1SideWalkThresholdRoadWidth = 2.0f;
    bool bAddSideWalk = true;
    bool bCheckMedian = true;
    bool bIgnoreHighway = true;
    bool bAddTrafficSignalLights = true;
    bool bSaveTmpData = false;
    bool bUseContourMesh = true;
    bool bMergeRoadGroup = true;
    bool bCalibrateIntersection = true;
    bool bSeparateContinuousBorder = true;

    RnRef_t<RnModel::CalibrateIntersectionBorderOption> CalibrateIntersectionOption;
    FRGraphFactory GraphFactory;
    struct FCreateRnModelRequest
    {
        APLATEAUInstancedCityModel* Actor;
        TWeakObjectPtr<USceneComponent> Transform;
        UStaticMesh* UnityMesh;
        TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
        //PLATEAURnStructureModel* OriginalMesh;
    };

    RnRef_t<RnModel> CreateRoadNetwork(const FCreateRnModelRequest& Req);

    RnRef_t<RnModel> CreateRoadNetwork(TSharedPtr<FRGraph> Graph);
};
