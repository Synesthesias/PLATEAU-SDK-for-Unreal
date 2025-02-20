// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/RoadMarking/PLATEAUMCIntersection.h"
#include "RoadAdjust/RoadNetworkToMesh/PLATEAURrTarget.h"
#include "RoadNetwork/Structure/RnIntersection.h"

FPLATEAUMarkedWayList UPLATEAUMCIntersection::ComposeFrom(const IPLATEAURrTarget* Target)
{
    FPLATEAUMarkedWayList Result;

    // 各交差点について処理
    const auto& Intersections = Target->GetIntersections();
    for (const auto& Inter : Intersections)
    {
        if (!Inter->IsValid())
            continue;

        // 交差点の境界のうち、他の道路を横切らない箇所に歩道の線を引きます
        const auto& Edges = Inter->GetEdges();
        for (const auto& Edge : Edges)
        {
            if (Edge->GetRoad() != nullptr)
                continue;

            const auto* Border = Edge->GetBorder();
            if (Border == nullptr || Border->CalcLength() > IntersectionLineIgnoreLength)
                continue; // 経験上、大きすぎる交差点は誤判定の可能性が高いので除外します

            Result.Add(FPLATEAUMarkedWay(
                FPLATEAUMWLine(Border->GetVertices()),
                EPLATEAUMarkedWayType::ShoulderLine,
                true /*方向は関係ない*/
            ));
        }
    }

    return Result;
}
