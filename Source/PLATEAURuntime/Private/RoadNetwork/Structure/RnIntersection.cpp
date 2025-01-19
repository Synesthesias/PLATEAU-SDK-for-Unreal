#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Util/VectorEx.h"


URnIntersectionEdge::URnIntersectionEdge()
{}

void URnIntersectionEdge::Init()
{}

bool URnIntersectionEdge::IsValid() const {
    return Road != nullptr && Border != nullptr;
}

bool URnIntersectionEdge::IsBorder() const
{
    return static_cast<bool>(Border);
}


FVector URnIntersectionEdge::GetCenterPoint() const {
    if (!Border) return FVector::ZeroVector;
    return Border->GetLerpPoint(0.5f);
}

// Add implementations:
bool URnIntersectionEdge::IsMedianBorder() const {
    if (IsBorder() == false)
        return false;

    if (!Road) 
        return false;
    if (auto R = Road->CastToRoad()) {
        auto Median = R->MedianLane;

        if (Median && Median->GetAllBorders().ContainsByPredicate([&](TRnRef_T<URnWay> b) {
            return b->IsSameLineReference(Border);
            })) {
            return true;
        }
    }
    return false;
}

TArray<TRnRef_T<URnLane>> URnIntersectionEdge::GetConnectedLanes() const {
    TArray<TRnRef_T<URnLane>> Lanes;
    if (!Road) return Lanes;
    if (auto R = Road->CastToRoad()) 
    {
        auto RoadLanes = R->GetAllLanesWithMedian();
        for (const auto& Lane : RoadLanes) {
            if (Lane->PrevBorder && Lane->PrevBorder->IsSameLineReference(Border)) {
                Lanes.Add(Lane);
            }
            if (Lane->NextBorder && Lane->NextBorder->IsSameLineReference(Border)) {
                Lanes.Add(Lane);
            }
        }
        return Lanes;
    }
    return Lanes;
}

TRnRef_T<URnLane> URnIntersectionEdge::GetConnectedLane(const TRnRef_T<URnWay>& BorderWay) const {
    if (!Road || !BorderWay) return nullptr;

    if (auto R = Road->CastToRoad()) 
    {
        auto RoadLanes = R->GetAllLanesWithMedian();
        for (const auto& Lane : RoadLanes) {
            if (Lane->PrevBorder && Lane->PrevBorder->IsSameLineReference(BorderWay)) {
                return Lane;
            }
            if (Lane->NextBorder && Lane->NextBorder->IsSameLineReference(BorderWay)) {
                return Lane;
            }
        }
        return nullptr;
    }
    return nullptr;
}

URnIntersection::URnIntersection()
{
}

void URnIntersection::Init(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran)
{
    if (TargetTran) {
        GetTargetTrans().Add(TargetTran);
    }
}

void URnIntersection::Init(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& InTargetTrans)
{
    for (auto Trans : InTargetTrans) {
        if (Trans) {
            GetTargetTrans().Add(Trans);
        }
    }
}

bool URnIntersection::IsValid() const {
    return Edges.Num() > 0;
}

TArray<TRnRef_T<URnIntersectionEdge>> URnIntersection::GetNeighbors() const {
    TArray<TRnRef_T<URnIntersectionEdge>> Result;
    for (auto& Edge : GetEdges()) {
        if (Edge->IsBorder())
            Result.Add(Edge);
    }
    return Result;
}

TArray<TRnRef_T<URnIntersectionEdge>> URnIntersection::GetEdgesBy(const TRnRef_T<URnRoadBase>& Road) const {
    return GetEdgesBy([&Road](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->Road == Road; });
}

TRnRef_T<URnIntersectionEdge> URnIntersection::GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const
{
    auto E = GetEdgesBy([&](const TRnRef_T<URnIntersectionEdge>& Edge) {
        return Edge->Road == Road && Edge->Border->IsSameLineReference(Border);
        });
    return E.Num() > 0 ? E[0] : nullptr;
}

TRnRef_T<URnIntersectionEdge> URnIntersection::GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnPoint>& Point) const {
    auto E = GetEdgesBy([&](const TRnRef_T<URnIntersectionEdge>& Edge) {
        return Edge->Road == Road && Edge->Border->LineString->GetPoints().Contains(Point);
        });
    return E.Num() > 0 ? E[0] : nullptr;
}


TArray<TRnRef_T<URnIntersectionEdge>> URnIntersection::GetEdgesBy(const TFunction<bool(const TRnRef_T<URnIntersectionEdge>&)>& Predicate) const {
    TArray<TRnRef_T<URnIntersectionEdge>> Result;
    for (const auto& Edge : Edges) {
        if (Predicate(Edge)) {
            Result.Add(Edge);
        }
    }
    return Result;
}

void URnIntersection::RemoveEdge(const TRnRef_T<URnRoad>& Road, const TRnRef_T<URnLane>& Lane)
{
    RemoveEdges([&](TRnRef_T<URnIntersectionEdge> Edge) 
        {
            if (Edge->Road != Road)
                return false;
            if (Lane->PrevBorder && Lane->PrevBorder->IsSameLineReference(Edge->Border))
                return true;
            if (Lane->NextBorder && Lane->NextBorder->IsSameLineReference(Edge->Border))
                return true;
            return false;
        });
}

void URnIntersection::RemoveEdges(const TRnRef_T<URnRoadBase>& Road) {
    RemoveEdges([&](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->Road == Road; });
}

void URnIntersection::RemoveEdges(const TFunction<bool(const TRnRef_T<URnIntersectionEdge>&)>& Predicate) {
    for (int32 i = Edges.Num() - 1; i >= 0; --i) {
        if (Predicate((Edges)[i])) {
            Edges.RemoveAt(i);
        }
    }
}

void URnIntersection::ReplaceEdges(const TRnRef_T<URnRoadBase>& Road, const TArray<TRnRef_T<URnWay>>& NewBorders) {
    RemoveEdges(Road);
    for (const auto& Border : NewBorders) {
        AddEdge(Road, Border);
    }
}

void URnIntersection::AddEdge(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) {
    auto NewEdge = RnNew<URnIntersectionEdge>();
    NewEdge->Road = Road;
    NewEdge->Border = Border;
    Edges.Add(NewEdge);
}

bool URnIntersection::HasEdge(const TRnRef_T<URnRoadBase>& Road) const {
    return GetEdgesBy(Road).Num() > 0;
}

bool URnIntersection::HasEdge(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const {
    return GetEdgeBy(Road, Border) != nullptr;
}

TArray<TRnRef_T<URnRoadBase>> URnIntersection::GetNeighborRoads() const {
    TArray<TRnRef_T<URnRoadBase>> Roads;
    for (const auto& Edge : Edges) {
        if (Edge->Road) {
            Roads.AddUnique(Edge->Road);
        }
    }
    return Roads;
}

TArray<TRnRef_T<URnWay>> URnIntersection::GetBorders() const {
    TArray<TRnRef_T<URnWay>> Borders;
    for (const auto& Edge : Edges) {
        if (Edge->Border) {
            Borders.Add(Edge->Border);
        }
    }
    return Borders;
}

void URnIntersection::UnLink(const TRnRef_T<URnRoadBase>& Other) {
    RemoveEdges(Other);
}

void URnIntersection::DisConnect(bool RemoveFromModel) {
    Super::DisConnect(RemoveFromModel);
    if (RemoveFromModel && GetParentModel()) {
        GetParentModel()->RemoveIntersection(TRnRef_T<URnIntersection>(this));
    }

    TArray<TRnRef_T<URnRoadBase>> Roads = GetNeighborRoads();
    for (const auto& Road : Roads) {
        Road->UnLink(TRnRef_T<URnRoadBase>(this));
    }
    Edges.Empty();
}

void URnIntersection::ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) {
    for (auto& Edge : Edges) {
        if (Edge->Road == From) {
            Edge->Road = To;
        }
    }
}

FVector URnIntersection::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    for (const auto& Edge : Edges) {
        if (Edge->Border) {
            Points.Add(Edge->Border->GetLerpPoint(0.5f));
        }
    }
    return FVectorEx::Centroid(Points);
}

TArray<TRnRef_T<URnWay>> URnIntersection::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways = Super::GetAllWays();
    for (const auto& Edge : Edges) {
        if (Edge->Border) {
            Ways.Add(Edge->Border);
        }
    }
    return Ways;
}


TRnRef_T<URnIntersection> URnIntersection::Create(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran) {
    return RnNew<URnIntersection>(TargetTran);
}

TRnRef_T<URnIntersection> URnIntersection::Create(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans) {
    return RnNew<URnIntersection>(TargetTrans);
}
