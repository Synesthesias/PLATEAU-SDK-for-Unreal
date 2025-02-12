#include "RoadNetwork/Structure/RnIntersection.h"

#include "Algo/AnyOf.h"
#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnTrackBuilder.h"
#include "RoadNetwork/Util/PLATEAUVectorEx.h"



URnIntersectionEdge::URnIntersectionEdge()
{}

void URnIntersectionEdge::Init()
{}

void URnIntersectionEdge::Init(URnRoadBase* InRoad, URnWay* InBorder)
{
    Road = InRoad;
    Border = InBorder;
}

bool URnIntersectionEdge::IsValid() const {
    return Road != nullptr && Border != nullptr;
}

bool URnIntersectionEdge::IsBorder() const
{
    return Road != nullptr;
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

ERnFlowTypeMask URnIntersectionEdge::GetFlowType() const
{
    if (Border == nullptr || Road == nullptr)
        return ERnFlowTypeMask::Empty;
    if (Border->IsValid() == false)
        return ERnFlowTypeMask::Empty;

    if (auto road = Road->CastToRoad()) 
    {
        auto ret = ERnFlowTypeMask::Empty;
        auto Lanes = road->GetConnectedLanes(Border);

        for(auto L : Lanes)
        {
            if (L->IsMedianLane())
                continue;
            if (L->GetNextBorder() && L->GetNextBorder()->IsSameLineReference(Border))
                ret |= ERnFlowTypeMask::Inbound;
            if (L->GetPrevBorder() && L->GetPrevBorder()->IsSameLineReference(Border))
                ret |= ERnFlowTypeMask::Outbound;
        }
        return ret;
    }
    // 交差点同士の接続の場合はレーン全部対象
    else if (auto intersection = Road->CastToIntersection()) {
        return ERnFlowTypeMask::Inbound | ERnFlowTypeMask::Outbound;
    }

    return ERnFlowTypeMask::Empty;
}

URnTrack::URnTrack()
    : FromBorder(nullptr)
    , ToBorder(nullptr)
    , Spline(nullptr)
    , TurnType(ERnTurnType::Straight) {
}

void URnTrack::Init(URnWay* InFromBorder, URnWay* InToBorder, USplineComponent* InSpline, ERnTurnType InTurnType) {
    FromBorder = InFromBorder;
    ToBorder = InToBorder;
    Spline = InSpline;
    TurnType = InTurnType;
}

bool URnTrack::IsSameInOut(const URnWay* OtherFromBorder, const URnWay* OtherToBorder) const {
    // RnWay クラスに IsSameLineReference() 関数が実装されている前提
    if (FromBorder && OtherFromBorder && ToBorder && OtherToBorder) {
        return FromBorder->IsSameLineReference(OtherFromBorder) && ToBorder->IsSameLineReference(OtherToBorder);
    }
    return false;
}

bool URnTrack::IsSameInOutWithTrack(const URnTrack* Other) const {
    if (Other) {
        return IsSameInOut(Other->FromBorder, Other->ToBorder);
    }
    return false;
}

bool URnTrack::ContainsBorder(const URnWay* Way) const {
    if (FromBorder && ToBorder && Way) {
        return FromBorder->IsSameLineReference(Way) || ToBorder->IsSameLineReference(Way);
    }
    return false;
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

int32 URnIntersection::ReplaceEdgeLink(TRnRef_T<URnWay> Border, TRnRef_T<URnRoadBase> AfterRoad)
{
    int32 Count = 0;
    for (auto& Edge : Edges) {
        if (Edge->GetBorder()->IsSameLineReference(Border)) {
            Edge->SetRoad(AfterRoad);
            Count++;
        }
    }
    return Count;
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
                Swap(Edges[i + 1], Edges[j]);
                break;
            }
            if (URnPoint::Equals(e0->GetPoint(-1), e1->GetPoint(-1))) {
                e1->Reverse(false);
                Swap(Edges[i + 1], Edges[j]);
                break;
            }
        }
    }

    // 時計回りになるように整列
    if (FGeoGraph2D::IsClockwise<TRnRef_T<URnIntersectionEdge>>(Edges
        , [](TRnRef_T<URnIntersectionEdge> E)
        {
            return FPLATEAURnDef::To2D(E->GetBorder()->GetVertex(0));
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

void URnIntersection::ClearTracks()
{
    Tracks.Empty();
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
    return FPLATEAUVectorEx::Centroid(Points);
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

bool URnIntersection::TryAddOrUpdateTrack(TRnRef_T<URnTrack> track)
{
    if (!track)
        return false;
    // trackの入口/出口がこの交差点のものかチェックする
    auto hasFrom = false;
    auto hasTo = false;
    for(auto E : Edges)
    {
        if (!E->GetBorder())
            continue;
        if (E->GetBorder()->IsSameLineReference(track->FromBorder))
            hasFrom = true;
        if (E->GetBorder()->IsSameLineReference(track->ToBorder))
            hasTo = true;
    }

    if (!hasFrom || !hasTo) {
        UE_LOG(LogTemp, Error, TEXT("交差点に含まれないトラックが追加されようとしています"));
        return false;
    }

    // それに同じ物が入口/出口のものがあれば削除する
    Tracks.RemoveAll([&](URnTrack* t) {
        return t->IsSameInOutWithTrack(track);
        });

    // track追加
    Tracks.Add(track);
    return true;
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

TArray<URnIntersectionEdge*> FRnIntersectionEx::FEdgeGroup::GetInBoundEdges() const
{
    TArray<URnIntersectionEdge*> InBoundEdges;
    for (auto& Edge : Edges) {
        if (Edge->GetBorder() && EnumHasAnyFlags (Edge->GetFlowType() , ERnFlowTypeMask::Inbound)) 
        {
            InBoundEdges.Add(Edge);
        }
    }
    return InBoundEdges;
        
}

TArray<URnIntersectionEdge*> FRnIntersectionEx::FEdgeGroup::GetOutBoundEdges() const
{
    TArray<URnIntersectionEdge*> InBoundEdges;
    for (auto& Edge : Edges) {
        if (Edge->GetBorder() && EnumHasAnyFlags(Edge->GetFlowType(), ERnFlowTypeMask::Outbound)) {
            InBoundEdges.Add(Edge);
        }
    }
    return InBoundEdges;
}

FVector FRnIntersectionEx::FEdgeGroup::GetNormal() const
{
    for(auto E : Edges)
    {
        if(FRnWayEx::IsValidWayOrDefault(E->GetBorder()))
        {
            return E->GetBorder()->GetEdgeNormal(0);
        }
    }
    return FVector::ZeroVector;
}

TArray<FRnIntersectionEx::FEdgeGroup> FRnIntersectionEx::CreateEdgeGroup(TRnRef_T<URnIntersection> Intersection)
{
    Intersection->Align();
    auto CopiedEdges = Intersection->GetEdges();
    auto Groups = FPLATEAURnEx::GroupByOutlineEdges<TRnRef_T<URnRoadBase>, TRnRef_T<URnIntersectionEdge>>(
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


    return Ret;
}

FVector FRnIntersectionEx::GetEdgeNormal(TRnRef_T<URnIntersectionEdge> Edge)
{
    URnIntersection::AlignEdgeNormal(Edge);
    auto Border = Edge->GetBorder();
    return Border->GetEdgeNormal((Border->Count() - 1) / 2);
}

FVector2D FRnIntersectionEx::GetEdgeNormal2D(TRnRef_T<URnIntersectionEdge> Edge)
{
    return FPLATEAURnDef::To2D(GetEdgeNormal(Edge));
}

FVector FRnIntersectionEx::GetEdgeCenter(TRnRef_T<URnIntersectionEdge> Edge)
{
    return Edge->GetBorder()->GetLerpPoint(0.5f);
}

FVector2D FRnIntersectionEx::GetEdgeCenter2D(TRnRef_T<URnIntersectionEdge> Edge)
{
    return FPLATEAURnDef::To2D(GetEdgeCenter(Edge));
}

void URnIntersection::SeparateContinuousBorder() {
    Align();

    for (int32 i = 0; i < Edges.Num(); ++i) {
        URnIntersectionEdge* E0 = Edges[i];
        URnIntersectionEdge* E1 = Edges[(i + 1) % Edges.Num()];

        if (E0->IsBorder() && E1->IsBorder() && E0->GetRoad() != E1->GetRoad()) {
            URnPoint* P0 = E0->GetBorder()->GetPoint(-1);
            URnPoint* P1 = E1->GetBorder()->GetPoint(0);

            // Offset by 1cm
            const float Offset = 0.01f * FPLATEAURnDef::Meter2Unit;

            if (E0->GetBorder()->Count() < 2 || E1->GetBorder()->Count() < 2) {
                UE_LOG(LogTemp, Error, TEXT("Border line has less than 2 vertices"));
                continue;
            }

            URnPoint* NewP0 = RnNew<URnPoint>(E0->GetBorder()->GetAdvancedPoint(Offset, true));
            URnPoint* NewP1 = RnNew<URnPoint>(E1->GetBorder()->GetAdvancedPoint(Offset, false));

            TArray<URnPoint*> Points = { NewP0, P0, NewP1 };
            URnLineString* LineString = URnLineString::Create(Points);

            auto AdjustPoint = [](URnIntersectionEdge* E, URnPoint* OldPoint, URnPoint* NewPoint) {
                E->GetBorder()->LineString->ReplacePoint(OldPoint, NewPoint);

                if (E->GetRoad()) {
                    TSet<URnLineString*> LineStrings = E->GetRoad()->GetAllLineStringsDistinct();
                    for (URnLineString* Ls : LineStrings) {
                        Ls->ReplacePoint(OldPoint, NewPoint);
                    }
                }
                };

            AdjustPoint(E0, P0, NewP0);
            AdjustPoint(E1, P1, NewP1);

            URnWay* Way = RnNew<URnWay>(LineString, false, true);

            URnIntersectionEdge* NewNeighbor = RnNew<URnIntersectionEdge>(nullptr, Way);
            Edges.Insert(NewNeighbor, i + 1);
            i++;
        }
    }
}

void URnIntersection::BuildTracks()
{
    Align();
    FRnTracksBuilder builder;
    builder.BuildTracks(this, FBuildTrackOption::Default());
}
