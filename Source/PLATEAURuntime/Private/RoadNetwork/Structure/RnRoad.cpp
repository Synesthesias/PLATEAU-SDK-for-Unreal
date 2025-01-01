#include "RoadNetwork/Structure/RnRoad.h"

#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Util/VectorEx.h"

RnRoad::RnRoad()
{
    MainLanes = MakeShared<TArray<RnRef_t<RnLane>>>();
}

RnRoad::RnRoad(UPLATEAUCityObjectGroup* TargetTran)
{
    MainLanes = MakeShared<TArray<RnRef_t<RnLane>>>();
    if (TargetTran) {
        TargetTrans->Add(TargetTran);
    }
}

RnRoad::RnRoad(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans)
{
    MainLanes = MakeShared<TArray<RnRef_t<RnLane>>>();
    for (auto* Trans : InTargetTrans) {
        if (Trans) {
            TargetTrans->Add(Trans);
        }
    }
}

TArray<RnRef_t<RnLane>> RnRoad::GetAllLanes() const {
    return *MainLanes;
}

TArray<RnRef_t<RnLane>> RnRoad::GetAllLanesWithMedian() const {
    TArray<RnRef_t<RnLane>> Lanes = *MainLanes;
    if (MedianLane) {
        Lanes.Add(MedianLane);
    }
    return Lanes;
}

bool RnRoad::IsValid() const {
    return MainLanes->Num() > 0;
}

bool RnRoad::IsAllBothConnectedLane() const {
    return std::all_of(MainLanes->begin(), MainLanes->end(), [](const RnRef_t<RnLane>& Lane) { return Lane->IsBothConnectedLane(); });
}

bool RnRoad::IsAllLaneValid() const {
    return std::all_of(MainLanes->begin(), MainLanes->end(), [](const RnRef_t<RnLane>& Lane) { return Lane->IsValidWay(); });
}

bool RnRoad::HasBothLane() const {
    return GetLeftLaneCount() > 0 && GetRightLaneCount() > 0;
}

bool RnRoad::IsEmptyRoad() const {
    return std::all_of(MainLanes->begin(), MainLanes->end(), [](const RnRef_t<RnLane>& Lane) { return Lane->IsEmptyLane(); });
}

TArray<RnRef_t<RnLane>> RnRoad::GetLanes(ERnDir Dir) const {
    return Dir == ERnDir::Left ? GetLeftLanes() : GetRightLanes();
}

bool RnRoad::IsLeftLane(const RnRef_t<RnLane>& Lane) const {
    return Lane && !Lane->IsReverse;
}

bool RnRoad::IsRightLane(const RnRef_t<RnLane>& Lane) const {
    return Lane && Lane->IsReverse;
}

ERnDir RnRoad::GetLaneDir(const RnRef_t<RnLane>& Lane) const {
    return IsLeftLane(Lane) ? ERnDir::Left : ERnDir::Right;
}

TArray<RnRef_t<RnWay>> RnRoad::GetBorders() const
{
    TArray<RnRef_t<RnWay>> Borders;
    for (const auto& Lane : GetAllLanesWithMedian()) {
        if (Lane->PrevBorder) {
            Borders.Add(Lane->PrevBorder);
        }
        if (Lane->NextBorder) {
            Borders.Add(Lane->NextBorder);
        }
    }
    return Borders;
}

TArray<RnRef_t<RnLane>> RnRoad::GetLeftLanes() const {
    return MainLanes->FilterByPredicate([this](const RnRef_t<RnLane>& Lane) { return IsLeftLane(Lane); });
}

TArray<RnRef_t<RnLane>> RnRoad::GetRightLanes() const {
    return MainLanes->FilterByPredicate([this](const RnRef_t<RnLane>& Lane) { return IsRightLane(Lane); });
}

int32 RnRoad::GetLeftLaneCount() const {
    return GetLeftLanes().Num();
}

int32 RnRoad::GetRightLaneCount() const {
    return GetRightLanes().Num();
}

float RnRoad::GetMedianWidth() const {
    return MedianLane ? MedianLane->CalcWidth() : 0.0f;
}

void RnRoad::SetMedianLane(const RnRef_t<RnLane>& Lane) {
    MedianLane = Lane;
    if (MedianLane) {
        MedianLane->Parent = RnRef_t<RnRoad>(this);
    }
}

bool RnRoad::IsMedianLane(const RnRef_t<const RnLane>& Lane) const {
    return MedianLane == Lane;
}

TArray<RnRef_t<RnRoadBase>> RnRoad::GetNeighborRoads() const {
    TArray<RnRef_t<RnRoadBase>> Roads;
    if (Prev) Roads.Add(Prev);
    if (Next) Roads.Add(Next);
    return Roads;
}

RnRef_t<RnWay> RnRoad::GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const {
    auto Lanes = Dir.has_value() == false ? GetAllLanesWithMedian() : GetLanes(*Dir);
    if (Lanes.Num() == 0) return nullptr;

    auto Border = Lanes[0]->GetBorder(BorderType);
    if (!Border) return nullptr;

    auto MergedWay = Border->Clone(true);
    for (int32 i = 1; i < Lanes.Num(); ++i) {
        auto NextBorder = Lanes[i]->GetBorder(BorderType);
        if (!NextBorder) continue;

        MergedWay->AppendBack2LineString(NextBorder);
    }
    return MergedWay;
}

RnRef_t<RnWay> RnRoad::GetMergedSideWay(ERnDir Dir) const {
    RnRef_t<RnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(Dir, LeftWay, RightWay)) {
        return nullptr;
    }
    return Dir == ERnDir::Left ? LeftWay : RightWay;
}

bool RnRoad::TryGetMergedSideWay(std::optional<ERnDir>  Dir, RnRef_t<RnWay>& OutLeftWay, RnRef_t<RnWay>& OutRightWay) const {
    auto Lanes = Dir.has_value() == false ? GetAllLanesWithMedian() : GetLanes(*Dir);
    if (Lanes.Num() == 0) return false;

    OutLeftWay = Lanes[0]->LeftWay->Clone(true);
    OutRightWay = Lanes[Lanes.Num() - 1]->RightWay->Clone(true);

    for (int32 i = 1; i < Lanes.Num(); ++i) {
        OutLeftWay->AppendBack2LineString(Lanes[i]->LeftWay);
    }
    for (int32 i = 0; i < Lanes.Num() - 1; ++i) {
        OutRightWay->AppendBack2LineString(Lanes[i]->RightWay);
    }
    return true;
}

bool RnRoad::TryGetNearestDistanceToSideWays(const RnRef_t<RnLineString>& LineString, float& OutDistance) const {
    if (!LineString || !LineString->IsValid()) return false;

    RnRef_t<RnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) return false;

    float MinDistance = MAX_FLT;
    for (int32 i = 0; i < LineString->Points->Num(); ++i) {
        FVector Point = LineString->GetPoint(i)->Vertex;
        FVector Nearest;
        float PointIndex, Distance;

        LeftWay->GetNearestPoint(Point, Nearest, PointIndex, Distance);
        MinDistance = FMath::Min(MinDistance, Distance);

        RightWay->GetNearestPoint(Point, Nearest, PointIndex, Distance);
        MinDistance = FMath::Min(MinDistance, Distance);
    }

    OutDistance = MinDistance;
    return true;
}

void RnRoad::AlignLaneBorder() {
    for (const auto& Lane : GetAllLanesWithMedian()) {
        if (!Lane->IsReverse) {
            if (Lane->PrevBorder) Lane->PrevBorder->IsReversed = false;
            if (Lane->NextBorder) Lane->NextBorder->IsReversed = true;
        }
        else {
            if (Lane->PrevBorder) Lane->PrevBorder->IsReversed = true;
            if (Lane->NextBorder) Lane->NextBorder->IsReversed = false;
        }
    }
}
bool RnRoad::TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const {
    RnRef_t<RnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) return false;

    int32 Index = BorderType == ERnLaneBorderType::Prev ? 0 : -1;
    OutSegment = FLineSegment3D(LeftWay->GetPoint(Index)->Vertex, RightWay->GetPoint(Index)->Vertex);
    return true;
}

RnRef_t<RnWay> RnRoad::GetBorderWay(const RnRef_t<RnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const {
    if (!Lane) return nullptr;

    auto Border = Lane->GetBorder(BorderType);
    if (!Border) return nullptr;

    return Dir == ERnLaneBorderDir::Left2Right ? Border : Border->ReversedWay();
}

void RnRoad::ReplaceLanes(const TArray<RnRef_t<RnLane>>& NewLanes, ERnDir Dir) {
    auto OldLanes = GetLanes(Dir);
    MainLanes = MakeShared<TArray<RnRef_t<RnLane>>>();

    for (const auto& Lane : GetLanes( FRnDirEx::GetOpposite(Dir))) {
        MainLanes->Add(Lane);
    }

    for (const auto& Lane : NewLanes) {
        Lane->Parent = RnRef_t<RnRoad>(this);
        MainLanes->Add(Lane);
    }
}

void RnRoad::ReplaceLanes(const TArray<RnRef_t<RnLane>>& NewLanes) {
    MainLanes = MakeShared<TArray<RnRef_t<RnLane>>>();
    for (const auto& Lane : NewLanes) {
        Lane->Parent = RnRef_t<RnRoad>(this);
        MainLanes->Add(Lane);
    }
}

void RnRoad::SetPrevNext(const RnRef_t<RnRoadBase>& PrevRoad, const RnRef_t<RnRoadBase>& NextRoad) {
    Prev = PrevRoad;
    Next = NextRoad;
}

void RnRoad::Reverse() {
    Swap(Prev, Next);
    for (auto& Lane : GetAllLanesWithMedian()) {
        Lane->Reverse();
    }
}

FVector RnRoad::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    RnRef_t<RnWay> LeftWay, RightWay;
    if (TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) {
        Points.Add(LeftWay->GetLerpPoint(0.5f));
        Points.Add(RightWay->GetLerpPoint(0.5f));
    }
    return FVectorEx::Centroid(Points);
}

TArray<RnRef_t<RnWay>> RnRoad::GetAllWays() const {
    TArray<RnRef_t<RnWay>> Ways = Super::GetAllWays();
    for (const auto& Lane : GetAllLanesWithMedian()) {
        for (const auto& Way : Lane->GetAllWays()) {
            Ways.Add(Way);
        }
    }
    return Ways;
}

void RnRoad::UnLink(const RnRef_t<RnRoadBase>& Other) {
    if (Prev == Other) Prev = nullptr;
    if (Next == Other) Next = nullptr;
}

void RnRoad::DisConnect(bool RemoveFromModel) {
    Super::DisConnect(RemoveFromModel);
    if (RemoveFromModel && ParentModel) {
        ParentModel->RemoveRoad(RnRef_t<RnRoad>(this));
    }
    if (Prev) Prev->UnLink(RnRef_t<RnRoadBase>(this));
    if (Next) Next->UnLink(RnRef_t<RnRoadBase>(this));
    Prev = nullptr;
    Next = nullptr;
}

void RnRoad::ReplaceNeighbor(const RnRef_t<RnRoadBase>& From, const RnRef_t<RnRoadBase>& To) {
    if (Prev == From) Prev = To;
    if (Next == From) Next = To;
}

RnRef_t<RnRoad> RnRoad::Create(UPLATEAUCityObjectGroup* TargetTran) {
    return RnNew<RnRoad>(TargetTran);
}

RnRef_t<RnRoad> RnRoad::Create(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans) {
    return RnNew<RnRoad>(TargetTrans);
}
TArray<RnRef_t<RnWay>> RnRoad::GetMergedSideWays() const {
    TArray<RnRef_t<RnWay>> Ways;
    RnRef_t<RnWay> LeftWay, RightWay;

    if (TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) 
    {
        Ways.Add(LeftWay);
        Ways.Add(RightWay);
    }
    return Ways;
}