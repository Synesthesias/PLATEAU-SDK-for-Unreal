
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/RrTargetRoadBases.h"
//#include "RrRoadNetworkCopier.h"

URrTargetRoadBases::URrTargetRoadBases()
    : NetworkModel(nullptr)
{
}

void URrTargetRoadBases::Initialize(URnModel* InNetwork, const TArray<URnRoadBase*>& InRoadBases)
{
    NetworkModel = InNetwork;
    RoadBasesArray = InRoadBases;
}

TArray<URnRoadBase*> URrTargetRoadBases::RoadBases()
{
    return RoadBasesArray;
}

TArray<URnRoad*> URrTargetRoadBases::Roads()
{
    TArray<URnRoad*> OutRoads;
    for (auto* RoadBase : RoadBasesArray)
    {
        if (URnRoad* Road = Cast<URnRoad>(RoadBase))
        {
            OutRoads.Add(Road);
        }
    }
    return OutRoads;
}

TArray<URnIntersection*> URrTargetRoadBases::Intersections()
{
    TArray<URnIntersection*> OutIntersections;
    for (auto* RoadBase : RoadBasesArray)
    {
        if (URnIntersection* Intersection = Cast<URnIntersection>(RoadBase))
        {
            OutIntersections.Add(Intersection);
        }
    }
    return OutIntersections;
}

TScriptInterface<IIRrTarget> URrTargetRoadBases::Copy()
{
    //TMap<URnRoadBase*, URnRoadBase*> RoadSrcToDst;
    //RnModel* CopiedNetwork = URrRoadNetworkCopier::Copy(NetworkModel, RoadSrcToDst);
    //
    //TArray<URnRoadBase*> CopiedRoadBases;
    //for (auto* Source : RoadBasesArray)
    //{
    //    if (RoadSrcToDst.Contains(Source))
    //    {
    //        CopiedRoadBases.Add(RoadSrcToDst[Source]);
    //    }
    //}
    //
    //URrTargetRoadBases* NewTarget = NewObject<URrTargetRoadBases>();
    //NewTarget->Initialize(CopiedNetwork, CopiedRoadBases);
    //return NewTarget;
    return nullptr;
}
