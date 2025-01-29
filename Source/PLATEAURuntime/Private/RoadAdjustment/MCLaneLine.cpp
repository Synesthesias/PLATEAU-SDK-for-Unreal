// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/MCLaneLine.h"
#include "RoadAdjustment/IRrTarget.h"
#include "RoadAdjustment/MarkedWay.h"
#include "RoadAdjustment/MarkedWayList.h"
#include "RoadAdjustment/MWLine.h"

TSharedPtr<UMarkedWayList> UMCLaneLine::ComposeFrom(const TSharedPtr<IRrTarget>& Target)
{
    auto ReturnList = MakeShareable(NewObject<UMarkedWayList>());

    //for (const auto& Road : Target->GetRoads())
    //{
    //    const TArray<FCarLane>& CarLanes = Road->GetMainLanes();

    //    // Process all lanes except the first and last ones (non-roadside lanes)
    //    for (int32 i = 1; i < CarLanes.Num() - 1; i++)
    //    {
    //        ReturnList.Add(FMarkedWay(
    //            FMWLine(CarLanes[i].GetLeftWay()),
    //            EMarkedWayType::LaneLine,
    //            CarLanes[i].IsReverse()
    //        ));
    //    }
    //}

    return ReturnList;
}
