#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnRoadBase.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Util/PLATEAUVector2DEx.h"
#include "RoadNetwork/Util/PLATEAUVectorEx.h"

URnSideWalk::URnSideWalk()
    : LaneType(EPLATEAURnSideWalkLaneType::Undefined) {
}

void URnSideWalk::Init()
{}

TRnRef_T<URnRoadBase> URnSideWalk::GetParentRoad() const
{ return RnFrom(ParentRoad); }

TArray<TRnRef_T<URnWay>> URnSideWalk::GetSideWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    if (OutsideWay) Ways.Add(OutsideWay);
    if (InsideWay) Ways.Add(InsideWay);
    return Ways;
}

TArray<TRnRef_T<URnWay>> URnSideWalk::GetEdgeWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    if (StartEdgeWay) Ways.Add(StartEdgeWay);
    if (EndEdgeWay) Ways.Add(EndEdgeWay);
    return Ways;
}

TArray<TRnRef_T<URnWay>> URnSideWalk::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways = GetSideWays();
    Ways.Append(GetEdgeWays());
    return Ways;
}

bool URnSideWalk::IsValid() const {
    return InsideWay && InsideWay->IsValid() && OutsideWay && OutsideWay->IsValid();
}

EPLATEAURnSideWalkWayTypeMask URnSideWalk::GetValidWayTypeMask() const {
    EPLATEAURnSideWalkWayTypeMask Mask = EPLATEAURnSideWalkWayTypeMask::None;
    if (OutsideWay) Mask |= EPLATEAURnSideWalkWayTypeMask::Outside;
    if (InsideWay) Mask |= EPLATEAURnSideWalkWayTypeMask::Inside;
    if (StartEdgeWay) Mask |= EPLATEAURnSideWalkWayTypeMask::StartEdge;
    if (EndEdgeWay) Mask |= EPLATEAURnSideWalkWayTypeMask::EndEdge;
    return Mask;
}

void URnSideWalk::SetParent(const TRnRef_T<URnRoadBase>& InParent) {
    ParentRoad = InParent;
}

void URnSideWalk::UnLinkFromParent() {
    if (GetParentRoad()) {
        GetParentRoad()->RemoveSideWalk(TRnRef_T<URnSideWalk>(this));
    }
}

void URnSideWalk::SetSideWays(const TRnRef_T<URnWay>& InOutsideWay, const TRnRef_T<URnWay>& InInsideWay) {
    OutsideWay = InOutsideWay;
    InsideWay = InInsideWay;
    TryAlign();
}

void URnSideWalk::SetEdgeWays(const TRnRef_T<URnWay>& StartWay, const TRnRef_T<URnWay>& EndWay) {
    StartEdgeWay = StartWay;
    EndEdgeWay = EndWay;
    TryAlign();
}

void URnSideWalk::SetStartEdgeWay(const TRnRef_T<URnWay>& StartWay) {
    StartEdgeWay = StartWay;
}

void URnSideWalk::SetEndEdgeWay(const TRnRef_T<URnWay>& EndWay) {
    EndEdgeWay = EndWay;
}

void URnSideWalk::ReverseLaneType() {
    if (LaneType == EPLATEAURnSideWalkLaneType::LeftLane)
        LaneType = EPLATEAURnSideWalkLaneType::RightLane;
    else if (LaneType == EPLATEAURnSideWalkLaneType::RightLane)
        LaneType = EPLATEAURnSideWalkLaneType::LeftLane;
}

void URnSideWalk::TryAlign() {
    auto AlignWay = [this](const TRnRef_T<URnWay>& Way) {
        if (!Way || !Way->IsValid()) return;

        if (StartEdgeWay && StartEdgeWay->IsValid()) {
            auto St = Way->GetPoint(0);
            if (StartEdgeWay->LineString->GetPoints().ContainsByPredicate(
                [&St](const TRnRef_T<URnPoint>& P) { return P->IsSamePoint(St); })) {
                return;
            }

            auto En = Way->GetPoint(-1);
            if (StartEdgeWay->LineString->GetPoints().ContainsByPredicate(
                [&En](const TRnRef_T<URnPoint>& P) { return P->IsSamePoint(En); })) {
                Way->Reverse(true);
                return;
            }
        }

        if (EndEdgeWay && EndEdgeWay->IsValid()) {
            auto En = Way->GetPoint(-1);
            if (EndEdgeWay->LineString->GetPoints().ContainsByPredicate(
                [&En](const TRnRef_T<URnPoint>& P) { return P->IsSamePoint(En); })) {
                return;
            }

            auto St = Way->GetPoint(0);
            if (EndEdgeWay->LineString->GetPoints().ContainsByPredicate(
                [&St](const TRnRef_T<URnPoint>& P) { return P->IsSamePoint(St); })) {
                Way->Reverse(true);
                return;
            }
        }
        };

    AlignWay(InsideWay);
    AlignWay(OutsideWay);
}

TRnRef_T<URnSideWalk> URnSideWalk::Create(
    const TRnRef_T<URnRoadBase>& Parent,
    const TRnRef_T<URnWay>& OutsideWay,
    const TRnRef_T<URnWay>& InsideWay,
    const TRnRef_T<URnWay>& StartEdgeWay,
    const TRnRef_T<URnWay>& EndEdgeWay,
    EPLATEAURnSideWalkLaneType LaneType) {
    auto SideWalk = RnNew<URnSideWalk>();
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

FVector URnSideWalk::GetCentralVertex() const {
    if (!this) return FVector::ZeroVector;

    TArray<FVector> Points;
    for (const auto& Way : GetSideWays()) {
        Points.Add(Way->GetLerpPoint(0.5f));
    }
    return FPLATEAUVectorEx::Centroid(Points);
}

bool URnSideWalk::IsNeighboring(const TRnRef_T<URnSideWalk>& Other) const {
    return GetAllWays().ContainsByPredicate([&Other](const TRnRef_T<URnWay>& X) {
        return Other->GetAllWays().ContainsByPredicate([&X](const TRnRef_T<URnWay>& B) {
            return X->IsSameLineReference(B);
            });
        });
}

float URnSideWalk::CalcRoadProximityScore(const TRnRef_T<URnRoadBase>& Other) const {
    if (!Other || !this) return -1.0f;

    TArray<TRnRef_T<URnWay>> TargetWays;
    if (auto Road = Other->CastToRoad()) {
        TargetWays = Road->GetMergedSideWays();
    }
    else if (auto Intersection = Other->CastToIntersection()) {
        for (const auto& Edge : Intersection->GetEdges()) {
            TargetWays.Add(Edge->GetBorder());
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
