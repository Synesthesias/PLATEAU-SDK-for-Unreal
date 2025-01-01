#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Util/VectorEx.h"


RnNeighbor::RnNeighbor()
{}

bool RnNeighbor::IsValid() const {
    return Road != nullptr && Border != nullptr;
}

bool RnNeighbor::IsBorder() const
{
    return static_cast<bool>(Border);
}


FVector RnNeighbor::GetCenterPoint() const {
    if (!Border) return FVector::ZeroVector;
    return Border->GetLerpPoint(0.5f);
}

// Add implementations:
bool RnNeighbor::IsMedianBorder() const {
    if (IsBorder() == false)
        return false;

    if (!Road) 
        return false;
    if (auto R = RnCast<RnRoad>(Road)) {
        auto Median = R->MedianLane;

        if (Median && Median->GetAllBorders().ContainsByPredicate([&](RnRef_t<RnWay> b) {
            return b->IsSameLineReference(Border);
            })) {
            return true;
        }
    }
    return false;
}

TArray<RnRef_t<RnLane>> RnNeighbor::GetConnectedLanes() const {
    TArray<RnRef_t<RnLane>> Lanes;
    if (!Road) return Lanes;
    if (auto R = RnCast<RnRoad>(Road)) 
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
}

RnRef_t<RnLane> RnNeighbor::GetConnectedLane(const RnRef_t<RnWay>& BorderWay) const {
    if (!Road || !BorderWay) return nullptr;

    if (auto R = RnCast<RnRoad>(Road)) 
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
   
}

RnIntersection::RnIntersection()
    : bIsEmptyIntersection(false) {
    Edges = MakeShared<TArray<RnRef_t<RnNeighbor>>>();
}

RnIntersection::RnIntersection(UPLATEAUCityObjectGroup* TargetTran)
    : bIsEmptyIntersection(false) {
    Edges = MakeShared<TArray<RnRef_t<RnNeighbor>>>();
    if (TargetTran) {
        TargetTrans->Add(TargetTran);
    }
}

RnIntersection::RnIntersection(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans)
    : bIsEmptyIntersection(false) {
    Edges = MakeShared<TArray<RnRef_t<RnNeighbor>>>();
    for (auto* Trans : InTargetTrans) {
        if (Trans) {
            TargetTrans->Add(Trans);
        }
    }
}

bool RnIntersection::IsValid() const {
    return Edges->Num() > 0;
}

TArray<RnRef_t<RnNeighbor>> RnIntersection::GetNeighbors() const {
    return *Edges;
}

TArray<RnRef_t<RnNeighbor>> RnIntersection::GetEdgesBy(const RnRef_t<RnRoadBase>& Road) const {
    return GetEdgesBy([&Road](const RnRef_t<RnNeighbor>& Edge) { return Edge->Road == Road; });
}

RnRef_t<RnNeighbor> RnIntersection::GetEdgeBy(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border) const
{
    auto E = GetEdgesBy([&](const RnRef_t<RnNeighbor>& Edge) {
        return Edge->Road == Road && Edge->Border->IsSameLineReference(Border);
        });
    return E.Num() > 0 ? E[0] : nullptr;
}

RnRef_t<RnNeighbor> RnIntersection::GetEdgeBy(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnPoint>& Point) const {
    auto E = GetEdgesBy([&](const RnRef_t<RnNeighbor>& Edge) {
        return Edge->Road == Road && Edge->Border->LineString->Points->Contains(Point);
        });
    return E.Num() > 0 ? E[0] : nullptr;
}


TArray<RnRef_t<RnNeighbor>> RnIntersection::GetEdgesBy(const TFunction<bool(const RnRef_t<RnNeighbor>&)>& Predicate) const {
    TArray<RnRef_t<RnNeighbor>> Result;
    for (const auto& Edge : *Edges) {
        if (Predicate(Edge)) {
            Result.Add(Edge);
        }
    }
    return Result;
}

void RnIntersection::RemoveEdges(const RnRef_t<RnRoadBase>& Road) {
    RemoveEdges([&](const RnRef_t<RnNeighbor>& Edge) { return Edge->Road == Road; });
}

void RnIntersection::RemoveEdges(const TFunction<bool(const RnRef_t<RnNeighbor>&)>& Predicate) {
    for (int32 i = Edges->Num() - 1; i >= 0; --i) {
        if (Predicate((*Edges)[i])) {
            Edges->RemoveAt(i);
        }
    }
}

void RnIntersection::ReplaceEdges(const RnRef_t<RnRoadBase>& Road, const TArray<RnRef_t<RnWay>>& NewBorders) {
    RemoveEdges(Road);
    for (const auto& Border : NewBorders) {
        AddEdge(Road, Border);
    }
}

void RnIntersection::AddEdge(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border) {
    auto NewEdge = RnNew<RnNeighbor>();
    NewEdge->Road = Road;
    NewEdge->Border = Border;
    Edges->Add(NewEdge);
}

bool RnIntersection::HasEdge(const RnRef_t<RnRoadBase>& Road) const {
    return GetEdgesBy(Road).Num() > 0;
}

bool RnIntersection::HasEdge(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border) const {
    return GetEdgeBy(Road, Border) != nullptr;
}

TArray<RnRef_t<RnRoadBase>> RnIntersection::GetNeighborRoads() const {
    TArray<RnRef_t<RnRoadBase>> Roads;
    for (const auto& Edge : *Edges) {
        if (Edge->Road) {
            Roads.AddUnique(Edge->Road);
        }
    }
    return Roads;
}

TArray<RnRef_t<RnWay>> RnIntersection::GetBorders() const {
    TArray<RnRef_t<RnWay>> Borders;
    for (const auto& Edge : *Edges) {
        if (Edge->Border) {
            Borders.Add(Edge->Border);
        }
    }
    return Borders;
}

void RnIntersection::UnLink(const RnRef_t<RnRoadBase>& Other) {
    RemoveEdges(Other);
}

void RnIntersection::DisConnect(bool RemoveFromModel) {
    Super::DisConnect(RemoveFromModel);
    if (RemoveFromModel && ParentModel) {
        ParentModel->RemoveIntersection(RnRef_t<RnIntersection>(this));
    }

    TArray<RnRef_t<RnRoadBase>> Roads = GetNeighborRoads();
    for (const auto& Road : Roads) {
        Road->UnLink(RnRef_t<RnRoadBase>(this));
    }
    Edges->Empty();
}

void RnIntersection::ReplaceNeighbor(const RnRef_t<RnRoadBase>& From, const RnRef_t<RnRoadBase>& To) {
    for (auto& Edge : *Edges) {
        if (Edge->Road == From) {
            Edge->Road = To;
        }
    }
}

FVector RnIntersection::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    for (const auto& Edge : *Edges) {
        if (Edge->Border) {
            Points.Add(Edge->Border->GetLerpPoint(0.5f));
        }
    }
    return FVectorEx::Centroid(Points);
}

TArray<RnRef_t<RnWay>> RnIntersection::GetAllWays() const {
    TArray<RnRef_t<RnWay>> Ways = Super::GetAllWays();
    for (const auto& Edge : *Edges) {
        if (Edge->Border) {
            Ways.Add(Edge->Border);
        }
    }
    return Ways;
}


RnRef_t<RnIntersection> RnIntersection::Create(UPLATEAUCityObjectGroup* TargetTran) {
    return RnNew<RnIntersection>(TargetTran);
}

RnRef_t<RnIntersection> RnIntersection::Create(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans) {
    return RnNew<RnIntersection>(TargetTrans);
}
