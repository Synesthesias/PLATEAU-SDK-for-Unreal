// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"

#include "RoadAdjust/PLATEAURoadLineType.h"

FPLATEAUMWLine::FPLATEAUMWLine(const URnWay::VertexEnumerator& InPoints) {
    Points = InPoints.ToArray();
}

float FPLATEAUMWLine::SumDistance() const {
    if (Points.Num() == 0)
        return 0.0f;

    float Length = 0.0f;
    for (int32 i = 1; i < Points.Num(); i++) {
        Length += FVector::Distance(Points[i], Points[i - 1]);
    }
    return Length;
}

FPLATEAUMarkedWay::FPLATEAUMarkedWay(const FPLATEAUMWLine& InLine, EPLATEAUMarkedWayType InType, bool bInIsReversed)
    : Line(InLine)
    , Type(InType)
    , bIsReversed(bInIsReversed) {
}

EPLATEAURoadLineType FPLATEAUMarkedWay::GetRoadLineType() const
{
    switch (GetMarkedWayType())
    {
    case EPLATEAUMarkedWayType::LaneLine:
        return EPLATEAURoadLineType::DashedWhilteLine;
    case EPLATEAUMarkedWayType::ShoulderLine:
        return EPLATEAURoadLineType::WhiteLine;
    case EPLATEAUMarkedWayType::CenterLineOver6MWidth:
        return EPLATEAURoadLineType::WhiteLine;
    case EPLATEAUMarkedWayType::CenterLineNearIntersection:
        return EPLATEAURoadLineType::YellowLine;
    case EPLATEAUMarkedWayType::CenterLineUnder6MWidth:
        return EPLATEAURoadLineType::DashedWhilteLine;
    case EPLATEAUMarkedWayType::Crosswalk:
        return EPLATEAURoadLineType::Crossing;
    default:
        return EPLATEAURoadLineType::None;
    }
}

void FPLATEAUMarkedWay::Translate(const FVector& Diff) {
    TArray<FVector> NewPoints = Line.GetPoints();
    for (FVector& Point : NewPoints) {
        Point += Diff;
    }
    Line.SetPoints(NewPoints);
}

FPLATEAUMarkedWayList::FPLATEAUMarkedWayList(const TArray<FPLATEAUMarkedWay>& InWays) : Ways(InWays) {
}

void FPLATEAUMarkedWayList::Add(const FPLATEAUMarkedWay& Way) {
    Ways.Add(Way);
}

void FPLATEAUMarkedWayList::AddRange(const FPLATEAUMarkedWayList& WayList) {
    Ways.Append(WayList.GetMarkedWays());
}

void FPLATEAUMarkedWayList::Translate(const FVector& Diff) {
    for (FPLATEAUMarkedWay& Way : Ways) {
        Way.Translate(Diff);
    }
}
