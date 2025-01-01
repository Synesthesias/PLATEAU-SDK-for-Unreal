#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnRoadBase.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Util/Vector2DEx.h"
#include "RoadNetwork/Util/VectorEx.h"

RnSideWalk::RnSideWalk()
    : LaneType(ERnSideWalkLaneType::Undefined) {
}

TArray<RnRef_t<RnWay>> RnSideWalk::GetSideWays() const {
    TArray<RnRef_t<RnWay>> Ways;
    if (OutsideWay) Ways.Add(OutsideWay);
    if (InsideWay) Ways.Add(InsideWay);
    return Ways;
}

TArray<RnRef_t<RnWay>> RnSideWalk::GetEdgeWays() const {
    TArray<RnRef_t<RnWay>> Ways;
    if (StartEdgeWay) Ways.Add(StartEdgeWay);
    if (EndEdgeWay) Ways.Add(EndEdgeWay);
    return Ways;
}

TArray<RnRef_t<RnWay>> RnSideWalk::GetAllWays() const {
    TArray<RnRef_t<RnWay>> Ways = GetSideWays();
    Ways.Append(GetEdgeWays());
    return Ways;
}

bool RnSideWalk::IsValid() const {
    return InsideWay && InsideWay->IsValid() && OutsideWay && OutsideWay->IsValid();
}

ERnSideWalkWayTypeMask RnSideWalk::GetValidWayTypeMask() const {
    ERnSideWalkWayTypeMask Mask = ERnSideWalkWayTypeMask::None;
    if (OutsideWay) Mask |= ERnSideWalkWayTypeMask::Outside;
    if (InsideWay) Mask |= ERnSideWalkWayTypeMask::Inside;
    if (StartEdgeWay) Mask |= ERnSideWalkWayTypeMask::StartEdge;
    if (EndEdgeWay) Mask |= ERnSideWalkWayTypeMask::EndEdge;
    return Mask;
}

void RnSideWalk::SetParent(const RnRef_t<RnRoadBase>& Parent) {
    ParentRoad = Parent;
}

void RnSideWalk::UnLinkFromParent() {
    if (ParentRoad) {
        ParentRoad->RemoveSideWalk(RnRef_t<RnSideWalk>(this));
    }
}

void RnSideWalk::SetSideWays(const RnRef_t<RnWay>& InOutsideWay, const RnRef_t<RnWay>& InInsideWay) {
    OutsideWay = InOutsideWay;
    InsideWay = InInsideWay;
    TryAlign();
}

void RnSideWalk::SetEdgeWays(const RnRef_t<RnWay>& StartWay, const RnRef_t<RnWay>& EndWay) {
    StartEdgeWay = StartWay;
    EndEdgeWay = EndWay;
    TryAlign();
}

void RnSideWalk::SetStartEdgeWay(const RnRef_t<RnWay>& StartWay) {
    StartEdgeWay = StartWay;
}

void RnSideWalk::SetEndEdgeWay(const RnRef_t<RnWay>& EndWay) {
    EndEdgeWay = EndWay;
}

void RnSideWalk::ReverseLaneType() {
    if (LaneType == ERnSideWalkLaneType::LeftLane)
        LaneType = ERnSideWalkLaneType::RightLane;
    else if (LaneType == ERnSideWalkLaneType::RightLane)
        LaneType = ERnSideWalkLaneType::LeftLane;
}

void RnSideWalk::TryAlign() {
    auto AlignWay = [this](const RnRef_t<RnWay>& Way) {
        if (!Way || !Way->IsValid()) return;

        if (StartEdgeWay && StartEdgeWay->IsValid()) {
            auto St = Way->GetPoint(0);
            if (StartEdgeWay->LineString->Points->ContainsByPredicate(
                [&St](const RnRef_t<RnPoint>& P) { return P->IsSamePoint(St); })) {
                return;
            }

            auto En = Way->GetPoint(-1);
            if (StartEdgeWay->LineString->Points->ContainsByPredicate(
                [&En](const RnRef_t<RnPoint>& P) { return P->IsSamePoint(En); })) {
                Way->Reverse(true);
                return;
            }
        }

        if (EndEdgeWay && EndEdgeWay->IsValid()) {
            auto En = Way->GetPoint(-1);
            if (EndEdgeWay->LineString->Points->ContainsByPredicate(
                [&En](const RnRef_t<RnPoint>& P) { return P->IsSamePoint(En); })) {
                return;
            }

            auto St = Way->GetPoint(0);
            if (EndEdgeWay->LineString->Points->ContainsByPredicate(
                [&St](const RnRef_t<RnPoint>& P) { return P->IsSamePoint(St); })) {
                Way->Reverse(true);
                return;
            }
        }
        };

    AlignWay(InsideWay);
    AlignWay(OutsideWay);
}

RnRef_t<RnSideWalk> RnSideWalk::Create(
    const RnRef_t<RnRoadBase>& Parent,
    const RnRef_t<RnWay>& OutsideWay,
    const RnRef_t<RnWay>& InsideWay,
    const RnRef_t<RnWay>& StartEdgeWay,
    const RnRef_t<RnWay>& EndEdgeWay,
    ERnSideWalkLaneType LaneType) {
    auto SideWalk = RnNew<RnSideWalk>();
    SideWalk->ParentRoad = Parent;
    SideWalk->OutsideWay = OutsideWay;
    SideWalk->InsideWay = InsideWay;
    SideWalk->StartEdgeWay = StartEdgeWay;
    SideWalk->EndEdgeWay = EndEdgeWay;
    SideWalk->LaneType = LaneType;
    SideWalk->TryAlign();

    if (Parent) {
        Parent->AddSideWalk(SideWalk);
    }

    return SideWalk;
}

FVector RnSideWalk::GetCentralVertex() const {
    if (!this) return FVector::ZeroVector;

    TArray<FVector> Points;
    for (const auto& Way : GetSideWays()) {
        Points.Add(Way->GetLerpPoint(0.5f));
    }
    return FVectorEx::Centroid(Points);
}

bool RnSideWalk::IsNeighboring(const RnRef_t<RnSideWalk>& Other) const {
    return GetAllWays().ContainsByPredicate([&Other](const RnRef_t<RnWay>& X) {
        return Other->GetAllWays().ContainsByPredicate([&X](const RnRef_t<RnWay>& B) {
            return X->IsSameLineReference(B);
            });
        });
}

float RnSideWalk::CalcRoadProximityScore(const RnRef_t<RnRoadBase>& Other) const {
    if (!Other || !this) return -1.0f;

    TArray<RnRef_t<RnWay>> TargetWays;
    if (auto Road = RnCast<RnRoad>(Other)) {
        TargetWays = Road->GetMergedSideWays();
    }
    else if (auto Intersection = RnCast<RnIntersection>(Other)) {
        for (const auto& Edge : Intersection->GetEdges()) {
            TargetWays.Add(Edge->Border);
        }
    }

    if (TargetWays.Num() == 0) return -1.0f;

    TArray<float> Distances;
    float SumDistance = 0.0f;
    int Num = 0;
    for (const auto& Point : InsideWay->GetVertices()) {
        float MinDist = MAX_FLT;
        for (const auto& Way : TargetWays) {
            FVector Nearest;
            float PointIndex, Distance;
            Way->GetNearestPoint(Point, Nearest, PointIndex, Distance);
            MinDist = FMath::Min(MinDist, Distance);
        }
        Distances.Add(MinDist);
        SumDistance += MinDist;
        Num++;
    }
    if (Num > 0) {
        return SumDistance / Num;
    }

    return -1.f;
}
