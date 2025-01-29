// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/MCShoulderLine.h"
#include "RoadAdjustment/IRrTarget.h"
#include "RoadAdjustment/MarkedWay.h"
#include "RoadAdjustment/MarkedWayList.h"
#include "RoadAdjustment/MWLine.h"

TSharedPtr<UMarkedWayList> UMCShoulderLine::ComposeFrom(const TSharedPtr<IRrTarget>& Target)
{
    auto ReturnList = MakeShareable(NewObject<UMarkedWayList>());
    //for (const auto& Road : Target->GetRoads())
    //{
    //    const TArray<TSharedPtr<FCarLane>>& CarLanes = Road->GetMainLanes();
    //    if (CarLanes.Num() == 0)
    //    {
    //        continue;
    //    }

    //    // For edge lanes, their LeftWay is between the sidewalk and roadway
    //    const TSharedPtr<FCarLane>& FirstLane = CarLanes[0];
    //    const TSharedPtr<FCarLane>& LastLane = CarLanes.Last();

    //    const TSharedPtr<FWay>& FirstLeft = FirstLane->GetLeftWay();
    //    const TSharedPtr<FWay>& LastLeft = LastLane->GetLeftWay();

    //    if (FirstLeft.IsValid())
    //    {
    //        ReturnList.Add(FMarkedWay(
    //            FMWLine(FirstLane->GetLeftWay()),
    //            EMarkedWayType::ShoulderLine,
    //            FirstLane->IsReverse()
    //        ));
    //    }

    //    if (LastLeft.IsValid())
    //    {
    //        ReturnList.Add(FMarkedWay(
    //            FMWLine(LastLane->GetLeftWay()),
    //            EMarkedWayType::ShoulderLine,
    //            LastLane->IsReverse()
    //        ));
    //    }
    //}

    return ReturnList;
}
