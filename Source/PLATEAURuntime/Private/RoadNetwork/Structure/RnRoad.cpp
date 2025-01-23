#include "RoadNetwork/Structure/RnRoad.h"

#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Util/VectorEx.h"

URnRoad::URnRoad()
{
}

URnRoad::URnRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran)
{
}

URnRoad::URnRoad(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& InTargetTrans)
{
}

void URnRoad::Init(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran)
{
    MainLanes.Reset();
    if (TargetTran) {
        GetTargetTrans().Add(TargetTran);
    }
}

void URnRoad::Init(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& InTargetTrans)
{
    MainLanes.Reset();
    for (auto&& Trans : InTargetTrans) {
        if (Trans) {
            GetTargetTrans().Add(Trans);
        }
    }
}

const TArray<TRnRef_T<URnLane>>& URnRoad::GetAllLanes() const {
    return MainLanes;
}

TArray<TRnRef_T<URnLane>> URnRoad::GetAllLanesWithMedian() const {
    TArray<TRnRef_T<URnLane>> Lanes = MainLanes;
    if (MedianLane) {
        Lanes.Add(MedianLane);
    }
    return Lanes;
}

bool URnRoad::IsValid() const {
    return MainLanes.Num() > 0;
}

bool URnRoad::IsAllBothConnectedLane() const {
    return std::all_of(MainLanes.begin(), MainLanes.end(), [](const TRnRef_T<URnLane>& Lane) { return Lane->IsBothConnectedLane(); });
}

bool URnRoad::IsAllLaneValid() const {
    return std::all_of(MainLanes.begin(), MainLanes.end(), [](const TRnRef_T<URnLane>& Lane) { return Lane->IsValidWay(); });
}

bool URnRoad::HasBothLane() const {
    return GetLeftLaneCount() > 0 && GetRightLaneCount() > 0;
}

bool URnRoad::IsEmptyRoad() const {
    return std::all_of(MainLanes.begin(), MainLanes.end(), [](const TRnRef_T<URnLane>& Lane) { return Lane->IsEmptyLane(); });
}

TArray<TRnRef_T<URnLane>> URnRoad::GetLanes(ERnDir Dir) const {
    return Dir == ERnDir::Left ? GetLeftLanes() : GetRightLanes();
}

bool URnRoad::IsLeftLane(const TRnRef_T<URnLane>& Lane) const {
    return Lane && !Lane->IsReverse;
}

bool URnRoad::IsRightLane(const TRnRef_T<URnLane>& Lane) const {
    return Lane && Lane->IsReverse;
}

ERnDir URnRoad::GetLaneDir(const TRnRef_T<URnLane>& Lane) const {
    return IsLeftLane(Lane) ? ERnDir::Left : ERnDir::Right;
}

TArray<TRnRef_T<URnWay>> URnRoad::GetBorders() const
{
    TArray<TRnRef_T<URnWay>> Borders;
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

TArray<TRnRef_T<URnLane>> URnRoad::GetLeftLanes() const {
    return MainLanes.FilterByPredicate([this](const TRnRef_T<URnLane>& Lane) { return IsLeftLane(Lane); });
}

TArray<TRnRef_T<URnLane>> URnRoad::GetRightLanes() const {
    return MainLanes.FilterByPredicate([this](const TRnRef_T<URnLane>& Lane) { return IsRightLane(Lane); });
}

int32 URnRoad::GetLeftLaneCount() const {
    return GetLeftLanes().Num();
}

int32 URnRoad::GetRightLaneCount() const {
    return GetRightLanes().Num();
}

float URnRoad::GetMedianWidth() const {
    return MedianLane ? MedianLane->CalcWidth() : 0.0f;
}

void URnRoad::SetMedianLane(const TRnRef_T<URnLane>& Lane) {
    MedianLane = Lane;
    if (MedianLane) {
        MedianLane->Parent = TRnRef_T<URnRoad>(this);
    }
}

bool URnRoad::IsMedianLane(const TRnRef_T<const URnLane>& Lane) const {
    return MedianLane == Lane;
}

TArray<TRnRef_T<URnRoadBase>> URnRoad::GetNeighborRoads() const {
    TArray<TRnRef_T<URnRoadBase>> Roads;
    if (Prev) Roads.Add(Prev);
    if (Next) Roads.Add(Next);
    return Roads;
}

void URnRoad::AddMainLane(TRnRef_T<URnLane> Lane)
{
    if (MainLanes.Contains(Lane))
        return;
    OnAddLane(Lane);
    MainLanes.Add(Lane);
}

TRnRef_T<URnWay> URnRoad::GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const {
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

TRnRef_T<URnWay> URnRoad::GetMergedSideWay(ERnDir Dir) const {
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(Dir, LeftWay, RightWay)) {
        return nullptr;
    }
    return Dir == ERnDir::Left ? LeftWay : RightWay;
}

bool URnRoad::TryGetMergedSideWay(std::optional<ERnDir>  Dir, TRnRef_T<URnWay>& OutLeftWay, TRnRef_T<URnWay>& OutRightWay) const
{
    auto Lanes = Dir.has_value() == false ? GetAllLanesWithMedian() : GetLanes(*Dir);
    if (Lanes.Num() == 0) return false;

    OutLeftWay = OutRightWay = nullptr;
    if (IsValid() == false)
        return false;

    TArray<TRnRef_T<URnLane>> TargetLanes;
    for (auto&& Lane : MainLanes) 
    {
        if (Dir.has_value() == false || GetLaneDir(Lane) == *Dir)
            TargetLanes.Add(Lane);
    }
    if (TargetLanes.IsEmpty())
        return false;

    auto LeftLane = TargetLanes[0];
    OutLeftWay = IsLeftLane(LeftLane) ? LeftLane->LeftWay : LeftLane->RightWay->ReversedWay();
    if(LeftLane)
    {
        if (IsLeftLane(LeftLane))
            OutLeftWay = LeftLane->LeftWay;
        else if (LeftLane->RightWay)
            OutRightWay = LeftLane->RightWay->ReversedWay();
    }

    auto RightLane = TargetLanes[TargetLanes.Num() - 1];
    if(RightLane)
    {
        if(IsLeftLane(RightLane))
        {
            OutRightWay = RightLane->RightWay;
        }
        else if (RightLane->LeftWay) {
            OutRightWay = RightLane->LeftWay->ReversedWay();
        }
    }

    return true;
}

bool URnRoad::TryGetNearestDistanceToSideWays(const TRnRef_T<URnLineString>& LineString, float& OutDistance) const {
    if (!LineString || !LineString->IsValid()) return false;

    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) return false;

    float MinDistance = MAX_FLT;
    for (int32 i = 0; i < LineString->Count(); ++i) {
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

void URnRoad::AlignLaneBorder() {
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
bool URnRoad::TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const {
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) return false;

    int32 Index = BorderType == ERnLaneBorderType::Prev ? 0 : -1;
    OutSegment = FLineSegment3D(LeftWay->GetPoint(Index)->Vertex, RightWay->GetPoint(Index)->Vertex);
    return true;
}

TRnRef_T<URnWay> URnRoad::GetBorderWay(const TRnRef_T<URnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const {
    if (!Lane) return nullptr;

    auto Border = Lane->GetBorder(BorderType);
    if (!Border) return nullptr;

    return Dir == ERnLaneBorderDir::Left2Right ? Border : Border->ReversedWay();
}

void URnRoad::ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes, ERnDir Dir) {
    auto OldLanes = GetLanes(Dir);
    MainLanes.Reset();

    for (const auto& Lane : GetLanes( FRnDirEx::GetOpposite(Dir))) {
        MainLanes.Add(Lane);
    }

    for (const auto& Lane : NewLanes) {
        Lane->Parent = TRnRef_T<URnRoad>(this);
        MainLanes.Add(Lane);
    }
}

void URnRoad::ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes) {
    MainLanes.Reset();
    for (const auto& Lane : NewLanes) {
        Lane->Parent = TRnRef_T<URnRoad>(this);
        MainLanes.Add(Lane);
    }
}

void URnRoad::SetPrevNext(const TRnRef_T<URnRoadBase>& PrevRoad, const TRnRef_T<URnRoadBase>& NextRoad) {
    Prev = PrevRoad;
    Next = NextRoad;
}

void URnRoad::Reverse() {
    Swap(Prev, Next);
    for (auto& Lane : GetAllLanesWithMedian()) {
        Lane->Reverse();
    }
}

FVector URnRoad::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) {
        Points.Add(LeftWay->GetLerpPoint(0.5f));
        Points.Add(RightWay->GetLerpPoint(0.5f));
    }
    return FVectorEx::Centroid(Points);
}

TArray<TRnRef_T<URnWay>> URnRoad::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways = Super::GetAllWays();
    for (const auto& Lane : GetAllLanesWithMedian()) {
        for (const auto& Way : Lane->GetAllWays()) {
            Ways.Add(Way);
        }
    }
    return Ways;
}

void URnRoad::UnLink(const TRnRef_T<URnRoadBase>& Other) {
    if (Prev == Other) Prev = nullptr;
    if (Next == Other) Next = nullptr;
}

void URnRoad::DisConnect(bool RemoveFromModel) {
    Super::DisConnect(RemoveFromModel);
    if (RemoveFromModel && GetParentModel()) {
        GetParentModel()->RemoveRoad(TRnRef_T<URnRoad>(this));
    }
    if (Prev) Prev->UnLink(TRnRef_T<URnRoadBase>(this));
    if (Next) Next->UnLink(TRnRef_T<URnRoadBase>(this));
    Prev = nullptr;
    Next = nullptr;
}

void URnRoad::ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) {
    if (Prev == From) Prev = To;
    if (Next == From) Next = To;
}

TRnRef_T<URnRoad> URnRoad::Create(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran) {
    return RnNew<URnRoad>(TargetTran);
}

TRnRef_T<URnRoad> URnRoad::Create(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans) {
    return RnNew<URnRoad>(TargetTrans);
}

TRnRef_T<URnRoad> URnRoad::CreateIsolatedRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran, TRnRef_T<URnWay> Way)
{
    const auto Lane = URnLane::CreateOneWayLane(Way);
    auto Ret = RnNew<URnRoad>(TargetTran);
    Ret->AddMainLane(Lane);
    return Ret;
        
}

TRnRef_T<URnRoad> URnRoad::CreateOneLaneRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran, TRnRef_T<URnLane> Lane)
{
    auto Ret = RnNew<URnRoad>(TargetTran);
    Ret->AddMainLane(Lane);
    return Ret;
}

void URnRoad::OnAddLane(TRnRef_T<URnLane> lane)
{
    if (!lane)
        return;
    lane->Parent = TRnRef_T<URnRoad>(this);
}

TArray<TRnRef_T<URnWay>> URnRoad::GetMergedSideWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    TRnRef_T<URnWay> LeftWay, RightWay;

    if (TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) 
    {
        Ways.Add(LeftWay);
        Ways.Add(RightWay);
    }
    return Ways;
}
