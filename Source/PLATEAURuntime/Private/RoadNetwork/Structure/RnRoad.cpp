#include "RoadNetwork/Structure/RnRoad.h"

#include "Algo/AnyOf.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Util/PLATEAUVectorEx.h"

void URnRoad::SetNext(TRnRef_T<URnRoadBase> InNext)
{
    this->Next = InNext;
}

void URnRoad::SetPrev(TRnRef_T<URnRoadBase> InPrev)
{
    this->Prev = InPrev;
}

void URnRoad::SetMedianLane1(TRnRef_T<URnLane> InMedianLane)
{
    this->MedianLane = InMedianLane;
}

TRnRef_T<URnRoadBase> URnRoad::GetNext() const
{
    return RnFrom(Next);
}

TRnRef_T<URnRoadBase> URnRoad::GetPrev() const
{
    return RnFrom(Prev);
}

TRnRef_T<URnRoadBase> URnRoad::GetNeighborRoad(EPLATEAURnLaneBorderType BorderType) const {
    if (BorderType == EPLATEAURnLaneBorderType::Prev) {
        return GetPrev();
    }
    if (BorderType == EPLATEAURnLaneBorderType::Next) {
        return GetNext();
    }
    return nullptr;
}

TRnRef_T<URnLane> URnRoad::GetMedianLane() const
{
    return RnFrom(MedianLane);
}

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

TArray<TRnRef_T<URnLane>> URnRoad::GetLanes(EPLATEAURnDir Dir) const {
    return Dir == EPLATEAURnDir::Left ? GetLeftLanes() : GetRightLanes();
}

bool URnRoad::TryGetLanes(TOptional<EPLATEAURnDir> Dir, TArray<TRnRef_T<URnLane>>& OutLanes) const
{
    if (Dir.IsSet() == false) {
        OutLanes = MainLanes;
        return true;
    }

    if (EPLATEAURnDir::Left == *Dir) {
        OutLanes = GetLeftLanes();
        return true;
    }

    if (EPLATEAURnDir::Right == *Dir) {
        OutLanes = GetRightLanes();
        return true;
    }
    return false;
}


bool URnRoad::IsLeftLane(const TRnRef_T<URnLane>& Lane) const {
    return Lane && !Lane->GetIsReverse();
}

bool URnRoad::IsRightLane(const TRnRef_T<URnLane>& Lane) const {
    return Lane && Lane->GetIsReverse();
}

EPLATEAURnDir URnRoad::GetLaneDir(const TRnRef_T<URnLane>& Lane) const {
    return IsLeftLane(Lane) ? EPLATEAURnDir::Left : EPLATEAURnDir::Right;
}

TArray<TRnRef_T<URnWay>> URnRoad::GetBorders() const
{
    TArray<TRnRef_T<URnWay>> Borders;
    for (const auto& Lane : GetAllLanesWithMedian()) {
        if (Lane->GetPrevBorder()) {
            Borders.Add(Lane->GetPrevBorder());
        }
        if (Lane->GetNextBorder()) {
            Borders.Add(Lane->GetNextBorder());
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
        MedianLane->SetParent(RnFrom(this));
    }
}

bool URnRoad::IsMedianLane(const TRnRef_T<const URnLane>& Lane) const {
    return MedianLane == Lane;
}

TArray<TRnRef_T<URnRoadBase>> URnRoad::GetNeighborRoads() const {
    TArray<TRnRef_T<URnRoadBase>> Roads;
    if (GetPrev()) Roads.Add(GetPrev());
    if (GetNext()) Roads.Add(GetNext());
    return Roads;
}

void URnRoad::AddMainLane(TRnRef_T<URnLane> Lane)
{
    if (MainLanes.Contains(Lane))
        return;
    OnAddLane(Lane);
    MainLanes.Add(Lane);
}

TRnRef_T<URnWay> URnRoad::GetMergedBorder(EPLATEAURnLaneBorderType BorderType, TOptional<EPLATEAURnDir> Dir) const
{
    TArray<TRnRef_T<URnLane>> Lanes;
    if (TryGetLanes(Dir, Lanes) == false)
        return nullptr;
    if (Lanes.Num() == 0)
        return nullptr;

    auto Ls = RnNew<URnLineString>();
    for(auto&& Lane : Lanes)
    {
        auto Way = GetBorderWay(Lane, BorderType, EPLATEAURnLaneBorderDir::Left2Right);
        if (!Way)
            continue;
        for (auto&& Point : Way->GetPoints()) {
            Ls->AddPointOrSkip(Point);
        }
    }
    return RnNew<URnWay>(Ls);
}

TRnRef_T<URnWay> URnRoad::GetMergedSideWay(EPLATEAURnDir Dir) const {
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(Dir, LeftWay, RightWay)) {
        return nullptr;
    }
    return Dir == EPLATEAURnDir::Left ? LeftWay : RightWay;
}

bool URnRoad::TryGetMergedSideWay(TOptional<EPLATEAURnDir>  Dir, TRnRef_T<URnWay>& OutLeftWay, TRnRef_T<URnWay>& OutRightWay) const
{
    OutLeftWay = OutRightWay = nullptr;
    if (IsValid() == false)
        return false;

    TArray<TRnRef_T<URnLane>> TargetLanes;
    for (auto&& Lane : MainLanes) 
    {
        if (Dir.IsSet() == false || GetLaneDir(Lane) == *Dir)
            TargetLanes.Add(Lane);
    }
    if (TargetLanes.IsEmpty())
        return false;

    auto LeftLane = TargetLanes[0];
    OutLeftWay = IsLeftLane(LeftLane) ? LeftLane->GetLeftWay() : LeftLane->GetRightWay()->ReversedWay();
    if(LeftLane)
    {
        if (IsLeftLane(LeftLane))
            OutLeftWay = LeftLane->GetLeftWay();
        else if (LeftLane->GetRightWay())
            OutRightWay = LeftLane->GetRightWay()->ReversedWay();
    }

    auto RightLane = TargetLanes[TargetLanes.Num() - 1];
    if(RightLane)
    {
        if(IsLeftLane(RightLane))
        {
            OutRightWay = RightLane->GetRightWay();
        }
        else if (RightLane->GetLeftWay()) {
            OutRightWay = RightLane->GetLeftWay()->ReversedWay();
        }
    }

    return true;
}

bool URnRoad::TryGetNearestDistanceToSideWays(const TRnRef_T<URnLineString>& LineString, float& OutDistance) const {
    if (!LineString || !LineString->IsValid()) return false;

    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(NullOpt, LeftWay, RightWay)) return false;

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

void URnRoad::AlignLaneBorder(EPLATEAURnLaneBorderDir BorderDir)
{
    for (const auto& Lane : GetAllLanesWithMedian()) {
        Lane->AlignBorder(BorderDir);
    }
}

bool URnRoad::TryGetAdjustBorderSegment(EPLATEAURnLaneBorderType BorderType, FLineSegment3D& OutSegment) const {
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (!TryGetMergedSideWay(NullOpt, LeftWay, RightWay)) return false;

    int32 Index = BorderType == EPLATEAURnLaneBorderType::Prev ? 0 : -1;
    OutSegment = FLineSegment3D(LeftWay->GetPoint(Index)->Vertex, RightWay->GetPoint(Index)->Vertex);
    return true;
}

TRnRef_T<URnWay> URnRoad::GetBorderWay(const TRnRef_T<URnLane>& Lane, EPLATEAURnLaneBorderType BorderType, EPLATEAURnLaneBorderDir BorderDir) const {
    if (!Lane) return nullptr;

    if(IsLeftLane(Lane) == false)
    {
        BorderType = FPLATEAURnLaneBorderTypeEx::GetOpposite(BorderType);
        BorderDir = FPLATEAURnLaneBorderDirEx::GetOpposite(BorderDir);
    }

    auto Border = Lane->GetBorder(BorderType);
    if (!Border) 
        return nullptr;

    if (Lane->GetBorderDir(BorderType) != BorderDir)
        Border = Border->ReversedWay();

    return Border;
}

void URnRoad::ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes, EPLATEAURnDir Dir) {
    auto OldLanes = GetLanes(Dir);
    MainLanes.Reset();

    for (const auto& Lane : GetLanes( FPLATEAURnDirEx::GetOpposite(Dir))) {
        MainLanes.Add(Lane);
    }

    for (const auto& Lane : NewLanes) {
        Lane->SetParent(RnFrom(this));
        MainLanes.Add(Lane);
    }
}

void URnRoad::ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes) {
    MainLanes.Reset();
    for (const auto& Lane : NewLanes) {
        Lane->SetParent(RnFrom(this));
        MainLanes.Add(Lane);
    }
}

void URnRoad::SetPrevNext(const TRnRef_T<URnRoadBase>& PrevRoad, const TRnRef_T<URnRoadBase>& NextRoad) {
    Prev = PrevRoad;
    Next = NextRoad;
}

void URnRoad::Reverse(bool KeepOneLaneIsLeft)
{
    Swap(Prev, Next);

    // 親の向きが変わるので,レーンの親に対する向きも変わる
    // 各レーンのWayの向きは変えずにIsRevereだけ変える
    // 左車線/右車線の関係が変わるので配列の並びも逆にする
    for (auto& Lane : GetAllLanesWithMedian()) {
        Lane->SetIsReverse(!Lane->GetIsReverse());
    }
    Algo::Reverse(MainLanes);
    for (auto& Sw : GetSideWalks())
        Sw->ReverseLaneType();

    // １車線道路でかつその道路が右車線扱いになったら, 左車線になるようにレーンを反転させる
    if (KeepOneLaneIsLeft) {
        auto AllLaneWithMedian = GetAllLanesWithMedian();
        if (AllLaneWithMedian.Num() == 1 && GetLeftLaneCount() == 0) {
            for(auto Lane : AllLaneWithMedian)                
                Lane->Reverse();

            for(auto Sw : GetSideWalks())
                Sw->ReverseLaneType();
        }
    }
}

FVector URnRoad::GetCentralVertex() const {
    if (!IsValid()) return FVector::ZeroVector;

    TArray<FVector> Points;
    TRnRef_T<URnWay> LeftWay, RightWay;
    if (TryGetMergedSideWay(NullOpt, LeftWay, RightWay)) {
        if(LeftWay)
            Points.Add(LeftWay->GetLerpPoint(0.5f));
        if (RightWay)
            Points.Add(RightWay->GetLerpPoint(0.5f));
    }
    return FPLATEAUVectorEx::Centroid(Points);
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
        GetParentModel()->RemoveRoad(RnFrom(this));
    }
    if (GetPrev()) GetPrev()->UnLink(RnFrom(this));
    if (GetNext()) GetNext()->UnLink(RnFrom(this));
    Prev = nullptr;
    Next = nullptr;
}

void URnRoad::ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) {
    if (Prev == From) Prev = To;
    if (Next == From) Next = To;
}

bool URnRoad::Check() const
{
    for (auto BorderType : { EPLATEAURnLaneBorderType::Prev, EPLATEAURnLaneBorderType::Next }) {
        if (!FRnRoadEx::IsValidBorderAdjacentNeighbor(this, BorderType, true))
            return false;
    }

    return Super::Check();
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
    lane->SetParent(RnFrom(this));
}

bool FRnRoadEx::IsValidBorderAdjacentNeighbor(const URnRoad* Self, EPLATEAURnLaneBorderType BorderType,
    bool NoBorderIsTrue)
{
    if (!Self)
        return false;
    auto Neighbor = Self->GetNeighborRoad(BorderType);
    if (!Neighbor)
        return NoBorderIsTrue;
    auto Ways = Neighbor->GetBorders();

    const auto OppositeBorderType = FPLATEAURnLaneBorderTypeEx::GetOpposite(BorderType);
    for (auto Lane : Self->GetMainLanes()) 
    {
        auto LaneBorder = Self->GetBorderWay(Lane, BorderType, EPLATEAURnLaneBorderDir::Left2Right);        
        if (!LaneBorder)
        {
            if (NoBorderIsTrue)
                continue;
            return false;
        }

        auto OppositeLaneBorder = Self->GetBorderWay(Lane, OppositeBorderType, EPLATEAURnLaneBorderDir::Left2Right);


        // 隣接する道路に共通の境界線を持たない場合はfalse
        if (!Algo::AnyOf(Ways, [LaneBorder](const TRnRef_T<URnWay>& Way) { return Way->IsSameLineReference(LaneBorder); })) 
        {
            // 逆側のボーダーを持っている場合, バグでPrev/Nextが逆になっている
            auto IsRev = Algo::AnyOf(Ways
                , [OppositeLaneBorder](const TRnRef_T<URnWay>& Way) { return Way->IsSameLineReference(OppositeLaneBorder); });
            if(!NoBorderIsTrue || IsRev)
                return false;
        }
    }
    return true;
}

TArray<TRnRef_T<URnWay>> URnRoad::GetMergedSideWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    TRnRef_T<URnWay> LeftWay, RightWay;

    if (TryGetMergedSideWay(NullOpt, LeftWay, RightWay))
    {
        Ways.Add(LeftWay);
        Ways.Add(RightWay);
    }
    return Ways;
}
