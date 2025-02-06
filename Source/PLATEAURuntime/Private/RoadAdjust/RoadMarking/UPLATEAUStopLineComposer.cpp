// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/UPLATEAUStopLineComposer.h"
#include "RoadNetwork/Structure/RnIntersection.h"


FPLATEAUMarkedWayList UPLATEAUStopLineComposer::ComposeFrom(const IPLATEAURrTarget* Target)
{
    auto WayList = FPLATEAUMarkedWayList();

    const auto& Roads = Target->GetRoads();
    for (const auto& Road : Roads)
    {
        if (!Road->IsValid())
        {
            continue;
        }

        // 次のノードが交差点の場合、停止線を追加
        const auto& Next = Road->GetNext();
        if (Next != nullptr && Next->CastToIntersection() != nullptr)
        {
            const auto NextBorder = FPLATEAUMWLine(
                Road->GetMergedBorder(EPLATEAURnLaneBorderType::Next, EPLATEAURnDir::Left));
            AddStopLine(WayList, NextBorder);
        }

        // 前のノードが交差点の場合、停止線を追加
        const auto& Prev = Road->GetPrev();
        if (Prev != nullptr && Prev->CastToIntersection() != nullptr)
        {
            const auto PrevBorder = FPLATEAUMWLine(
                Road->GetMergedBorder(EPLATEAURnLaneBorderType::Prev, EPLATEAURnDir::Right));
            AddStopLine(WayList, PrevBorder);
        }
    }

    // 道路にめりこまないよう高さをオフセット
    WayList.Translate(FVector(0.0f, 0.0f, CONST_HeightOffset));
    return WayList;
}

void UPLATEAUStopLineComposer::AddStopLine(FPLATEAUMarkedWayList& WayList, const FPLATEAUMWLine& Border)
{
    WayList.Add(FPLATEAUMarkedWay(Border, EPLATEAUMarkedWayType::StopLine, false));
}
