#include "RoadNetwork/Structure/RnIntersection.h"

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
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

void URnIntersectionEdge::Init(TObjectPtr<URnRoadBase> InRoad, TObjectPtr<URnWay> InBorder)
{
    Road = InRoad;
    Border = InBorder;
}

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
            if (Lane->GetPrevBorder() && Lane->GetPrevBorder()->IsSameLineReference(Border)) {
                Lanes.Add(Lane);
            }
            if (Lane->GetNextBorder() && Lane->GetNextBorder()->IsSameLineReference(Border)) {
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
            if (Lane->GetPrevBorder() && Lane->GetPrevBorder()->IsSameLineReference(BorderWay)) {
                return Lane;
            }
            if (Lane->GetNextBorder() && Lane->GetNextBorder()->IsSameLineReference(BorderWay)) {
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
    return GetEdgesBy([&Road](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->GetRoad() == Road; });
}

TRnRef_T<URnIntersectionEdge> URnIntersection::GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const
{
    auto E = GetEdgesBy([&](const TRnRef_T<URnIntersectionEdge>& Edge) {
        return Edge->GetRoad() == Road && Edge->GetBorder()->IsSameLineReference(Border);
        });
    return E.Num() > 0 ? E[0] : nullptr;
}

TRnRef_T<URnIntersectionEdge> URnIntersection::GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnPoint>& Point) const {
    auto E = GetEdgesBy([&](const TRnRef_T<URnIntersectionEdge>& Edge) {
        return Edge->GetRoad() == Road && Edge->GetBorder()->LineString->GetPoints().Contains(Point);
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
            if (Edge->GetRoad() != Road)
                return false;
            if (Lane->GetPrevBorder() && Lane->GetPrevBorder()->IsSameLineReference(Edge->GetBorder()))
                return true;
            if (Lane->GetNextBorder() && Lane->GetNextBorder()->IsSameLineReference(Edge->GetBorder()))
                return true;
            return false;
        });
}

void URnIntersection::RemoveEdges(const TRnRef_T<URnRoadBase>& Road) {
    RemoveEdges([&](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->GetRoad() == Road; });
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
    auto NewEdge = RnNew<URnIntersectionEdge>(Road, Border);
    Edges.Add(NewEdge);
}

bool URnIntersection::HasEdge(const TRnRef_T<URnRoadBase>& Road) const {
    return GetEdgesBy(Road).Num() > 0;
}

bool URnIntersection::HasEdge(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const {
    return GetEdgeBy(Road, Border) != nullptr;
}

void URnIntersection::Align()
{
    for (auto&& i = 0; i < Edges.Num(); ++i) 
    {
        auto&& e0 = Edges[i]->GetBorder();
        for (auto&& j = i + 1; j < Edges.Num(); ++j) {
            auto&& e1 = Edges[j]->GetBorder();
            if (URnPoint::Equals(e0->GetPoint(-1), e1->GetPoint(0))) {
                (Edges[i + 1], Edges[j]) = (Edges[j], Edges[i + 1]);
                break;
            }
            if (URnPoint::Equals(e0->GetPoint(-1), e1->GetPoint(-1))) {
                e1->Reverse(false);
                (Edges[i + 1], Edges[j]) = (Edges[j], Edges[i + 1]);
                break;
            }
        }
    }

    // 時計回りになるように整列
    if (FGeoGraph2D::IsClockwise<TRnRef_T<URnIntersectionEdge>>(Edges
        , [](TRnRef_T<URnIntersectionEdge> E)
        {
            return FRnDef::To2D(E->GetBorder()->GetVertex(0));
        }) == false) 
    {
        for(auto&& e : Edges)
            e->GetBorder()->Reverse(true);
        Algo::Reverse(Edges);
    }

    // 法線は必ず外向きを向くようにする
    for(auto&& e : Edges) {
        AlignEdgeNormal(e);
    }
}

TArray<TRnRef_T<URnRoadBase>> URnIntersection::GetNeighborRoads() const {
    TArray<TRnRef_T<URnRoadBase>> Roads;
    for (const auto& Edge : Edges) {
        if (Edge->GetRoad()) {
            Roads.AddUnique(Edge->GetRoad());
        }
    }
    return Roads;
}

TArray<TRnRef_T<URnWay>> URnIntersection::GetBorders() const {
    TArray<TRnRef_T<URnWay>> Borders;
    for (const auto& Edge : Edges) {
        if (Edge->GetBorder()) {
            Borders.Add(Edge->GetBorder());
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
        if (Edge->GetRoad() == From) {
            Edge->SetRoad(To);
        }
    }
}

FVector URnIntersection::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    for (const auto& Edge : Edges) {
        if (Edge->GetBorder()) {
            Points.Add(Edge->GetBorder()->GetLerpPoint(0.5f));
        }
    }
    return FVectorEx::Centroid(Points);
}

TArray<TRnRef_T<URnWay>> URnIntersection::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways = Super::GetAllWays();
    for (const auto& Edge : Edges) {
        if (Edge->GetBorder()) {
            Ways.Add(Edge->GetBorder());
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

void URnIntersection::AlignEdgeNormal(TRnRef_T<URnIntersectionEdge> edge)
{
    if (edge && edge->GetBorder() && edge->GetBorder()->IsReverseNormal)
        edge->GetBorder()->IsReverseNormal = false;
}

bool FRnIntersectionEx::FEdgeGroup::IsValid() const
{
    if (Edges.IsEmpty())
        return false;
    for(auto&& E : Edges)
    {
        if (E && FRnWayEx::IsValidWayOrDefault(E->GetBorder()) == false)
            return false;
    }
    return true;
}

TArray<FRnIntersectionEx::FEdgeGroup> FRnIntersectionEx::CreateEdgeGroup(TRnRef_T<URnIntersection> Intersection)
{
    auto CopiedEdges = Intersection->GetEdges();
    auto Groups = FRnEx::GroupByOutlineEdges<TRnRef_T<URnRoadBase>, TRnRef_T<URnIntersectionEdge>>(
        CopiedEdges
        , [](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->GetRoad(); }
    );

    TArray<FEdgeGroup> Ret;
    Ret.SetNum(Groups.Num());
    for(auto i = 0; i < Groups.Num(); ++i)
    {
        auto& G = Groups[i];
        Ret[i].Key = G.Key;
        Ret[i].Edges = G.Edges;
        Ret[i].RightSide = &Ret[(i + Ret.Num() - 1) % Ret.Num()];
        Ret[i].LeftSide = &Ret[(i + 1) % Ret.Num()];
    }


    return {};
}
