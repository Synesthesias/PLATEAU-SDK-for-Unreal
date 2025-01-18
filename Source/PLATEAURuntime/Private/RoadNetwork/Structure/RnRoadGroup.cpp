#include "RoadNetwork/Structure/RnRoadGroup.h"

#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnLineString.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Util/RnEx.h"

URnRoadGroup::URnRoadGroup(TRnRef_T<URnIntersection> InPrevIntersection,
                         TRnRef_T<URnIntersection> InNextIntersection,
                         const TArray<TRnRef_T<URnRoad>>& InRoads)
{
    Init(InPrevIntersection, InNextIntersection, InRoads);
}

void URnRoadGroup::Init(TRnRef_T<URnIntersection> InPrevIntersection, TRnRef_T<URnIntersection> InNextIntersection,
    const TArray<TRnRef_T<URnRoad>>& InRoads)
{
    PrevIntersection = InPrevIntersection;
    NextIntersection = InNextIntersection;
    Align();
}

bool URnRoadGroup::IsValid() const
{
    return Roads.Num() > 0 && std::all_of(Roads.begin(), Roads.end(), [](const TRnRef_T<URnRoad>& Road) { return Road->IsValid(); });
}

int32 URnRoadGroup::GetLeftLaneCount() const {
    Align();
    if (Roads.Num() == 0) return 0;

    auto Ret = INT_MAX;
    for (auto& Road : Roads) {
        Ret = FMath::Min(Ret, Road->GetLeftLaneCount());
    }
    return Ret;
}

int32 URnRoadGroup::GetRightLaneCount() const {
    Align();
    if (Roads.Num() == 0) return 0;

    auto Ret = INT_MAX;
    for (auto& Road : Roads) {
        Ret = FMath::Min(Ret, Road->GetRightLaneCount());
    }
    return Ret;
}

TArray<TRnRef_T<URnLane>> URnRoadGroup::GetRightLanes() const {
    Align();
    TArray<TRnRef_T<URnLane>> Result;
    for (const auto& Road : Roads) {
        Result.Append(Road->GetRightLanes());
    }
    return Result;
}

TArray<TRnRef_T<URnLane>> URnRoadGroup::GetLeftLanes() const {
    Align();
    TArray<TRnRef_T<URnLane>> Result;
    for (const auto& Road : Roads) {
        Result.Append(Road->GetLeftLanes());
    }
    return Result;
}

TArray<TRnRef_T<URnLane>> URnRoadGroup::GetLanes(ERnDir Dir) const {
    switch (Dir) {
    case ERnDir::Left:
        return GetLeftLanes();
    case ERnDir::Right:
        return GetRightLanes();
    default:
        checkf(false, TEXT("Invalid direction in GetLanes"));
        return TArray<TRnRef_T<URnLane>>();
    }
}

void URnRoadGroup::GetMedians(TArray<TRnRef_T<URnWay>>& OutLeft, TArray<TRnRef_T<URnWay>>& OutRight) const {
    OutLeft.Empty(Roads.Num());
    OutRight.Empty(Roads.Num());

    for (const auto& Road : Roads) {
        if (Road->GetLeftLaneCount() > 0) {
            auto CenterLeft = Road->GetLeftLanes().Last();
            OutLeft.Add(CenterLeft->RightWay);
        }
        if (Road->GetRightLaneCount() > 0) {
            auto CenterRight = Road->GetRightLanes()[0];
            OutRight.Add(CenterRight->RightWay);
        }
    }
}

bool URnRoadGroup::HasMedians() const {
    return Roads.ContainsByPredicate([](const TRnRef_T<URnRoad>& Road) { return Road->MedianLane != nullptr; });
}

bool URnRoadGroup::Align() const
{
    return const_cast<URnRoadGroup*>(this)->Align();
}

bool URnRoadGroup::CreateMedianOrSkip(float MedianWidth, float MaxMedianLaneRate) {
    if (HasMedian()) return false;

    if (GetLeftLaneCount() == 0 || GetRightLaneCount() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create median without both left and right lanes"));
        return false;
    }

    MedianWidth = FMath::Max(1.0f, MedianWidth);
    float Width = 0.0f;
    for (const auto& Road : Roads) {
        float RoadWidth = 0.0f;
        for (const auto& Lane : Road->GetAllLanesWithMedian()) {
            RoadWidth += Lane->CalcWidth();
        }
        Width = FMath::Min(Width, RoadWidth);
    }

    float MedianRate = FMath::Min(MaxMedianLaneRate, MedianWidth / Width);
    SetLaneCountWithMedian(GetLeftLaneCount(), GetRightLaneCount(), MedianRate);
    return true;
}

bool URnRoadGroup::HasMedian() const {
    return std::all_of(Roads.begin(), Roads.end(), [](const TRnRef_T<URnRoad>& Road) { return Road->MedianLane != nullptr; });
}

bool URnRoadGroup::IsAligned() const {
    if (Roads.Num() <= 1) return true;

    for (int32 i = 0; i < Roads.Num(); ++i) {
        if (i < Roads.Num() - 1 && (Roads)[i]->Next != (Roads)[i + 1])
            return false;

        if (i > 0 && (Roads)[i]->Prev != (Roads)[i - 1])
            return false;
    }

    return true;
}

bool URnRoadGroup::IsDeepAligned() const {
    if (!IsAligned()) return false;
    if (Roads.Num() <= 1) return true;

    for (int32 i = 1; i < Roads.Num(); ++i) {
        auto PrevRoad = (Roads)[i - 1];
        auto NowRoad = (Roads)[i];
        auto NowLanes = NowRoad->GetAllLanesWithMedian();
        auto PrevLanes = PrevRoad->GetAllLanesWithMedian();

        if (NowLanes.Num() != PrevLanes.Num()) {
            UE_LOG(LogTemp, Error, TEXT("Lane counts is mismatch"));
            return false;
        }

        for (int32 j = 0; j < FMath::Min(NowLanes.Num(), PrevLanes.Num()); ++j) {
            auto NowLane = NowLanes[j];
            auto PrevLane = PrevLanes[j];

            if (!PrevLane->IsReverse) {
                if (!NowLane->PrevBorder->IsSameLineReference(PrevLane->NextBorder)) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid Direction Lane[%d]"), j);
                    return false;
                }
            }
            else {
                if (!PrevLane->PrevBorder->IsSameLineReference(NowLane->NextBorder)) {
                    UE_LOG(LogTemp, Error, TEXT("Invalid Direction Lane[%d]"), j);
                    return false;
                }
            }
        }
    }

    return true;
}

bool URnRoadGroup::Align() {
    if (IsAligned()) return true;

    // Roads.Count <= 1の場合はIsAligned=trueなのでここでは
    // インデックス範囲外チェックはしなくてよい
    // 0番目が逆かどうかチェック
    if ((Roads)[0]->Next != (Roads)[1]) {
        (Roads)[0]->Reverse();
    }

    // 1番目以降が逆かどうかチェック
    for (int32 i = 1; i < Roads.Num(); ++i) {
        if ((Roads)[i]->Prev != (Roads)[i - 1]) {
            (Roads)[i]->Reverse();
        }
    }

    // 境界線の向きもそろえる
    for (const auto& Road : Roads) {
        Road->AlignLaneBorder();
    }

    if ((Roads)[0]->Prev != PrevIntersection) {
        Swap(PrevIntersection, NextIntersection);
    }

    return IsDeepAligned();
}

bool URnRoadGroup::MergeRoads() {
    if (!Align()) return false;
    if (Roads.Num() <= 1) return true;

    // マージ先の道路
    auto DstRoad = (Roads)[0];
    auto DstLanes = DstRoad->GetAllLanesWithMedian();
    auto& DstSideWalks = DstRoad->GetSideWalks();

    for (int32 i = 1; i < Roads.Num(); i++) {
        // マージ元の道路. DstRoadに統合されて消える
        auto SrcRoad = (Roads)[i];
        auto SrcLanes = SrcRoad->GetAllLanesWithMedian();

        // SideWalksと共通のLineStringがあるとき, レーン側は統合されるけど
        // SideWalksは統合されない場合もある. その時はLineStringを分離する必要があるので
        // 元のLineStringをコピーして持っておく
        auto OriginalDstSideWalks = DstSideWalks;
        TMap<TRnRef_T<URnWay>, TRnRef_T<URnWay>> Original;
        for (const auto& SideWalk : DstSideWalks) {
            for (const auto& Way : SideWalk->GetSideWays()) {
                if (Way) {
                    Original.Add(Way, Way->Clone(false));
                }
            }
        }

        // SideWalksと共通のLineStringもあるので2回追加しないように記録しておく
        TSet<TRnRef_T<URnLineString>> Visited;
        for (int32 j = 0; j < SrcLanes.Num(); ++j) {
            auto SrcLane = SrcLanes[j];
            auto DstLane = DstLanes[j];

            // 順方向(左車線)
            if (SrcRoad->IsLeftLane(SrcLane)) {
                DstLane->LeftWay->AppendBack2LineString(SrcLane->LeftWay);
                Visited.Add(DstLane->LeftWay->LineString);
                if (j == SrcLanes.Num() - 1) {
                    DstLane->RightWay->AppendBack2LineString(SrcLane->RightWay);
                    Visited.Add(DstLane->RightWay->LineString);
                }

                DstLane->SetBorder(ERnLaneBorderType::Next, SrcLane->NextBorder);
            }
            // 逆方向(右車線)
            else {
                DstLane->RightWay->AppendFront2LineString(SrcLane->RightWay);
                Visited.Add(DstLane->RightWay->LineString);
                if (j == SrcLanes.Num() - 1) {
                    DstLane->LeftWay->AppendFront2LineString(SrcLane->LeftWay);
                    Visited.Add(DstLane->LeftWay->LineString);
                }

                DstLane->SetBorder(ERnLaneBorderType::Prev, SrcLane->PrevBorder);
            }
        }

        auto SrcSideWalks = SrcRoad->GetSideWalks();
        TSet<TRnRef_T<URnSideWalk>> MergedDstSideWalks;

        for (const auto& SrcSw : SrcSideWalks) {
            bool Found = false;
            for (const auto& DstSw : DstSideWalks) {
                auto MergeSideWalk = [&](bool Reverse, const TFunction<void(TRnRef_T<URnWay>, TRnRef_T<URnWay>)>& Merger) {
                    auto InSideWay = Reverse ? SrcSw->GetInsideWay()->ReversedWay() : SrcSw->GetInsideWay();
                    auto OutsideWay = Reverse ? SrcSw->GetOutsideWay()->ReversedWay() : SrcSw->GetOutsideWay();

                    if (DstSw->GetInsideWay()) {
                        if (!Visited.Contains(DstSw->GetInsideWay()->LineString)) {
                            Merger(DstSw->GetInsideWay(), InSideWay);
                            Visited.Add(DstSw->GetInsideWay()->LineString);
                        }
                        InSideWay = DstSw->GetInsideWay();
                    }

                    if (DstSw->GetOutsideWay()) {
                        if (!Visited.Contains(DstSw->GetOutsideWay()->LineString)) {
                            Merger(DstSw->GetOutsideWay(), OutsideWay);
                            Visited.Add(DstSw->GetOutsideWay()->LineString);
                        }
                        OutsideWay = DstSw->GetOutsideWay();
                    }

                    DstSw->SetSideWays(OutsideWay, InSideWay);
                    MergedDstSideWalks.Add(DstSw);
                    Found = true;
                    };

                // start - startで重なっている場合
                if (DstSw->GetStartEdgeWay() && SrcSw->GetStartEdgeWay() && DstSw->GetStartEdgeWay()->IsSameLineReference(SrcSw->GetStartEdgeWay())) {
                    MergeSideWalk(true, [](TRnRef_T<URnWay> A, TRnRef_T<URnWay> B) { A->AppendFront2LineString(B); });
                    DstSw->SetStartEdgeWay(SrcSw->GetEndEdgeWay());
                }
                // start - endで重なっている場合
                else if (DstSw->GetStartEdgeWay() && SrcSw->GetEndEdgeWay() && DstSw->GetStartEdgeWay()->IsSameLineReference(SrcSw->GetEndEdgeWay())) {
                    MergeSideWalk(false, [](TRnRef_T<URnWay> A, TRnRef_T<URnWay> B) { A->AppendFront2LineString(B); });
                    DstSw->SetStartEdgeWay(SrcSw->GetStartEdgeWay());
                }
                // end - endで重なっている場合
                else if (DstSw->GetEndEdgeWay() && SrcSw->GetEndEdgeWay() && DstSw->GetEndEdgeWay()->IsSameLineReference(SrcSw->GetEndEdgeWay())) {
                    MergeSideWalk(true, [](TRnRef_T<URnWay> A, TRnRef_T<URnWay> B) { A->AppendBack2LineString(B); });
                    DstSw->SetEndEdgeWay(SrcSw->GetStartEdgeWay());
                }
                // end - startで重なっている場合
                else if (DstSw->GetEndEdgeWay() && SrcSw->GetStartEdgeWay() && DstSw->GetEndEdgeWay()->IsSameLineReference(SrcSw->GetStartEdgeWay())) {
                    MergeSideWalk(false, [](TRnRef_T<URnWay> A, TRnRef_T<URnWay> B) { A->AppendBack2LineString(B); });
                    DstSw->SetEndEdgeWay(SrcSw->GetEndEdgeWay());
                }

                if (Found) break;
            }

            // マージできなかった歩道は直接追加
            if (!Found) {
                SrcSw->SetSideWays(SrcSw->GetOutsideWay(), SrcSw->GetInsideWay());
                DstRoad->AddSideWalk(SrcSw);
                DstSideWalks.Add(SrcSw);
            }
        }

        // DstSideWalksの中でマージされなかった(元の形状から変更されない)ものは
        // レーンと共通のLineStringを持っている場合に勝手に形状変わっているかもしれないので明示的に元に戻す
        for (const auto& Sw : OriginalDstSideWalks) {
            if (!MergedDstSideWalks.Contains(Sw)) {
                Sw->SetSideWays(
                    Sw->GetOutsideWay() ? Original[Sw->GetOutsideWay()] : nullptr,
                    Sw->GetInsideWay() ? Original[Sw->GetInsideWay()] : nullptr);
            }
        }
        DstRoad->AddTargetTrans(SrcRoad->GetTargetTrans());
        SrcRoad->DisConnect(true);
    }

    if (NextIntersection) {
        NextIntersection->RemoveEdges([&](const TRnRef_T<URnIntersectionEdge>& Edge) { return Edge->Road == (Roads)[Roads.Num() - 1]; });
        for (const auto& Lane : DstLanes) {
            NextIntersection->AddEdge(DstRoad, DstRoad->GetBorderWay(Lane, ERnLaneBorderType::Next, ERnLaneBorderDir::Left2Right));
        }
    }

    DstRoad->SetPrevNext(PrevIntersection, NextIntersection);

    Roads.Reset();
    Roads.Add(DstRoad);
    return true;
}

TRnRef_T<URnRoadGroup> URnRoadGroup::CreateRoadGroupOrDefault(TRnRef_T<URnIntersection> PrevIntersection, TRnRef_T<URnIntersection> NextIntersection) {
    if (!PrevIntersection || !NextIntersection) return nullptr;

    for (const auto& Road : PrevIntersection->GetNeighborRoads()) {
        if (auto RoadPtr = Road->CastToRoad()) {
            TArray<TRnRef_T<URnRoad>> Roads = { RoadPtr };
            auto Ret = RnNew<URnRoadGroup>(PrevIntersection, NextIntersection, Roads);
            bool HasPrev = Ret->PrevIntersection == PrevIntersection || Ret->NextIntersection == PrevIntersection;
            bool HasNext = Ret->PrevIntersection == NextIntersection || Ret->NextIntersection == NextIntersection;
            if (HasPrev && HasNext) {
                return Ret;
            }
        }
    }

    return nullptr;
}

bool URnRoadGroup::IsSameRoadGroup(TRnRef_T<URnRoadGroup> A, TRnRef_T<URnRoadGroup> B) {
    if (!A && !B) return false;

    // 同じ交差点を含むか（Next,Prevは問わない）
    bool IsSameIntersection =
        (A->PrevIntersection == B->PrevIntersection && A->NextIntersection == B->NextIntersection) ||
        (A->PrevIntersection == B->NextIntersection && A->NextIntersection == B->PrevIntersection);

    // 同じ道路を含むか
    bool IsContainSameRoads = true;
    for (const auto& Road : A->Roads) {
        if (!B->Roads.Contains(Road)) {
            IsContainSameRoads = false;
            break;
        }
    }

    return IsSameIntersection && IsContainSameRoads;
}

void URnRoadGroup::AdjustBorder() {
    Align();

    auto Adjust = [](TRnRef_T<URnRoad> Road, ERnLaneBorderType BorderType, TRnRef_T<URnIntersection> Inter) {
        if (!Inter) return;

        FLineSegment3D BorderLeft2Right;
        if (!Road->TryGetAdjustBorderSegment(BorderType, BorderLeft2Right))
            return;

        auto LeftWay = Road->GetMergedSideWay(ERnDir::Left);
        auto RightWay = Road->GetMergedSideWay(ERnDir::Right);

        auto DuplicatePoints = [&Inter,&Road](TRnRef_T<URnPoint> P, const FVector& CheckVertex) {
            if ((P->Vertex - CheckVertex).SizeSquared() < 1e-6f)
                return;

            for (const auto& Edge : Inter->GetEdges()) {
                if (Edge->Road != Road && Edge->Border->LineString->Contains(P)) {
                    int32 I = Edge->Border->LineString->GetPoints().IndexOfByKey(P);
                    if (I == 0) {
                        Edge->Border->LineString->GetPoints().Insert(RnNew<URnPoint>(P->Vertex), 1);
                    }
                    else if (I == Edge->Border->LineString->GetPoints().Num() - 1) {
                        Edge->Border->LineString->GetPoints().Insert(RnNew<URnPoint>(P->Vertex), Edge->Border->LineString->GetPoints().Num() - 1);
                    }
                }
            }
            };

        int32 LastPointIndex = BorderType == ERnLaneBorderType::Prev ? 0 : -1;
        DuplicatePoints(LeftWay->GetPoint(LastPointIndex), BorderLeft2Right.GetStart());
        DuplicatePoints(RightWay->GetPoint(LastPointIndex), BorderLeft2Right.GetEnd());

        auto AdjustWay = [&](TRnRef_T<URnLane> Lane, TRnRef_T<URnWay> Way) {
            if (Lane->IsReverse)
                Way = Way->ReversedWay();
            auto P = Way->GetPoint(LastPointIndex);
            auto N = BorderLeft2Right.GetNearestPoint(P->Vertex);
            P->Vertex = N;
            };

        auto Lanes = Road->GetAllLanesWithMedian();
        TArray<TRnRef_T<URnWay>> NewBorders;
        NewBorders.Reserve(Lanes.Num());

        for (int32 i = 0; i < Lanes.Num(); ++i) {
            auto Lane = Lanes[i];
            AdjustWay(Lane, Lane->LeftWay);
            if (i == Lanes.Num() - 1)
                AdjustWay(Lane, Lane->RightWay);

            auto L = Lane->LeftWay;
            auto R = Lane->RightWay;
            if (!Road->IsLeftLane(Lane)) {
                L = L->ReversedWay();
                R = R->ReversedWay();
            }

            auto Points = TArray<TRnRef_T<URnPoint>>();
            Points.Add(L->GetPoint(LastPointIndex));
            Points.Add(R->GetPoint(LastPointIndex));

            auto Ls = RnNew<URnLineString>();
            Ls->SetPoints(Points);
            NewBorders.Add(RnNew<URnWay>(Ls));
        }

        Inter->ReplaceEdges(Road, NewBorders);
        for (int32 i = 0; i < Lanes.Num(); ++i) {
            auto B = NewBorders[i];
            auto Lane = Lanes[i];
            if (Road->IsLeftLane(Lane)) {
                Lane->SetBorder(BorderType, B);
            }
            else {
                Lane->SetBorder(  FRnLaneBorderTypeEx::GetOpposite(BorderType), B->ReversedWay());
            }
        }
        };

    Adjust((Roads)[Roads.Num() - 1], ERnLaneBorderType::Next, NextIntersection);
    Adjust((Roads)[0], ERnLaneBorderType::Prev, PrevIntersection);
}
void URnRoadGroup::SetLaneCountImpl(int32 Count, ERnDir Dir, bool RebuildTrack)
{
    if (IsValid() == false)
        return;

    // 既に指定の数になっている場合は何もしない
    if (std::all_of(Roads.begin(), Roads.end(), [Count, Dir](const TRnRef_T<URnRoad> R) {
        return  R->GetLanes(Dir).Num() == Count;
        })) {
        return;
    }

    const auto AfterLanes = SplitLane(Count, Dir);

    for(auto I = 0; I < Roads.Num(); ++I)
    {
        auto Road = (Roads)[I];
        auto Lanes =AfterLanes[Road];

        auto BeforeLanes = Road->GetLanes(Dir);
        if (I == Roads.Num() - 1 && NextIntersection) 
        {
            for(auto l : BeforeLanes)
                NextIntersection->RemoveEdge(Road, l);
            for(auto l : Lanes) 
            {
                NextIntersection->AddEdge(Road, l->NextBorder);
            }
        }
        if (I == 0 && PrevIntersection) 
        {
            for(auto l : BeforeLanes)
                PrevIntersection->RemoveEdge(Road, l);
            for(auto l : Lanes) {
                PrevIntersection->AddEdge(Road, l->PrevBorder);
            }
        }

        // 右車線の場合は反対にする
        // #NOTE : 隣接情報変更後に反転させる
        if (Dir == ERnDir::Right) 
        {
            for(auto l : Lanes)
                l->Reverse();
        }

        (Roads)[I]->ReplaceLanes(Lanes, Dir);
    }
}

void URnRoadGroup::SetLaneCountWithoutMedian(int32 LeftCount, int32 RightCount, bool RebuildTrack) {
    if (std::all_of(Roads.begin(), Roads.end(), [LeftCount, RightCount](const TRnRef_T<URnRoad>& Road) {
        return Road->GetLeftLaneCount() == LeftCount && Road->GetRightLaneCount() == RightCount;
        })) {
        return;
    }

    Align();

    auto Num = LeftCount + RightCount;
    auto AfterLanes = SplitLane(Num, std::nullopt);

    TArray<TRnRef_T<URnLineString>> NewNextBorders;
    TArray<TRnRef_T<URnLineString>> NewPrevBorders;

    for (int32 i = 0; i < Roads.Num(); ++i) {
        auto Road = (Roads)[i];
        auto& Lanes = AfterLanes[Road];

        if (i == Roads.Num() - 1) {
            NextIntersection->ReplaceEdges(Road, FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnWay>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->NextBorder; }));
            NewNextBorders.Append(FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnLineString>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->NextBorder->LineString; }));
        }

        if (i == 0) {
            PrevIntersection->ReplaceEdges(Road, FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnWay>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->PrevBorder; }));
            NewPrevBorders.Append(FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnLineString>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->PrevBorder->LineString; }));
        }

        for (int32 j = LeftCount; j < Lanes.Num(); ++j) {
            Lanes[j]->Reverse();
        }

        Road->ReplaceLanes(Lanes);
    }

    // 中央分離帯を削除する
    for (const auto& Road : Roads) {
        Road->SetMedianLane(nullptr);
    }
}

void URnRoadGroup::SetLaneCount(int32 LeftCount, int32 RightCount, bool RebuildTrack) {
    if (!IsValid()) return;

    if (LeftCount <= 0 && RightCount <= 0) {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set both lane counts to 0"));
        return;
    }

    Align();

    auto NowLeft = GetLeftLaneCount();
    auto NowRight = GetRightLaneCount();

    if ((NowLeft > 0 || LeftCount == 0) && (NowRight > 0 || RightCount == 0)) {
        SetLeftLaneCount(LeftCount, RebuildTrack);
        SetRightLaneCount(RightCount, RebuildTrack);
        return;
    }

    SetLaneCountWithoutMedian(LeftCount, RightCount, RebuildTrack);
}

void URnRoadGroup::SetLaneCountWithMedian(int32 LeftCount, int32 RightCount, float MedianWidthRate) {
    if (!IsValid()) return;

    if (LeftCount <= 0 && RightCount <= 0) {
        UE_LOG(LogTemp, Warning, TEXT("Cannot set both lane counts to 0"));
        return;
    }

    Align();

    auto Num = LeftCount + RightCount + 1;
    auto LaneRate = (1.0f - MedianWidthRate) / (Num - 1);

    auto AfterLanes = SplitLane(Num, std::nullopt, [LeftCount, LaneRate, MedianWidthRate](int32 I) {
        if (I == LeftCount)
            return MedianWidthRate;
        return LaneRate;
        });

    TArray<TRnRef_T<URnLineString>> NewNextBorders;
    TArray<TRnRef_T<URnLineString>> NewPrevBorders;

    for (int32 i = 0; i < Roads.Num(); ++i) {
        auto Road = (Roads)[i];
        auto Lanes = AfterLanes[Road];

        if (i == Roads.Num() - 1) 
        {
            NextIntersection->ReplaceEdges(Road, FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnWay>>( Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->NextBorder; }));
            NewNextBorders.Append( FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnLineString>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->NextBorder->LineString; }));
        }

        if (i == 0) {
            PrevIntersection->ReplaceEdges(Road, FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnWay>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->PrevBorder; }));
            NewPrevBorders.Append(FRnEx::Map<TRnRef_T<URnLane>, TRnRef_T<URnLineString>>(Lanes, [](const TRnRef_T<URnLane>& Lane) { return Lane->PrevBorder->LineString; }));
        }

        for (int32 j = LeftCount + 1; j < Lanes.Num(); ++j) {
            Lanes[j]->Reverse();
        }

        auto Median = Lanes[LeftCount];
        Lanes.RemoveAt(LeftCount);
        Road->ReplaceLanes(Lanes);
        Road->SetMedianLane(Median);
    }
}

void URnRoadGroup::SetLeftLaneCount(int32 Count, bool RebuildTrack) {
    if (GetLeftLaneCount() == Count) return;

    if (GetLeftLaneCount() == 0 || Count == 0) {
        SetLaneCountWithoutMedian(Count, GetRightLaneCount(), RebuildTrack);
    }
    else {
        SetLaneCountImpl(Count, ERnDir::Left, RebuildTrack);
    }
}

void URnRoadGroup::SetRightLaneCount(int32 Count, bool RebuildTrack) {
    if (GetRightLaneCount() == Count) return;

    if (GetRightLaneCount() == 0 || Count == 0) {
        SetLaneCountWithoutMedian(GetLeftLaneCount(), Count, RebuildTrack);
    }
    else {
        SetLaneCountImpl(Count, ERnDir::Right, RebuildTrack);
    }
}

void URnRoadGroup::SetLaneCount(ERnDir Dir, int32 Count, bool RebuildTrack) {
    switch (Dir) {
    case ERnDir::Left:
        SetLeftLaneCount(Count, RebuildTrack);
        break;
    case ERnDir::Right:
        SetRightLaneCount(Count, RebuildTrack);
        break;
    default:
        checkf(false, TEXT("Invalid direction in SetLaneCount"));
        break;
    }
}
TMap<TRnRef_T<URnRoad>, TArray<TRnRef_T<URnLane>>> URnRoadGroup::SplitLane(
    int32 Num,
    std::optional<ERnDir> Dir,
    TFunction<float(int32)> GetSplitRate) {
    if (Num <= 0) 
        return TMap<TRnRef_T<URnRoad>, TArray<TRnRef_T<URnLane>>>();

    auto Result = TMap<TRnRef_T<URnRoad>, TArray<TRnRef_T<URnLane>>>();

    auto MergedBorders = FRnEx::Map<TRnRef_T<URnRoad>, TRnRef_T<URnWay>>(Roads, [Dir](const TRnRef_T<URnRoad>& Road) {
        return Road->GetMergedBorder(ERnLaneBorderType::Prev, Dir);
        });
    MergedBorders.Add((Roads)[Roads.Num() - 1]->GetMergedBorder(ERnLaneBorderType::Next, Dir));

    TArray<TArray<TRnRef_T<URnWay>>> BorderWays;
    BorderWays.Reserve(Roads.Num() + 1);

    for (const auto& B : MergedBorders) {
        auto Split = B->Split(Num, false, GetSplitRate);
        BorderWays.Add(Split);
    }

    for (int32 i = 0; i < Roads.Num(); ++i) {
        auto Road = (Roads)[i];
        auto PrevBorders = BorderWays[i];
        auto NextBorders = BorderWays[i + 1];

        TRnRef_T<URnWay> LeftWay, RightWay;
        Road->TryGetMergedSideWay(Dir, LeftWay, RightWay);

        auto LeftVertices = LeftWay->GetVertices().ToArray();
        auto RightVertices = RightWay->GetVertices().ToArray();
        auto Left = LeftWay;
        TArray<TRnRef_T<URnLane>> Lanes;
        Lanes.Reserve(Num);
        float Rate = 0.0f;

        for (int32 N = 0; N < Num; ++N) {
            auto Right = RightWay;
            if (N < Num - 1) {
                Rate += GetSplitRate ? GetSplitRate(N) : (1.0f / Num);
                auto PrevBorder = PrevBorders[N];
                auto NextBorder = NextBorders[N];
                auto Line = FRnEx::CreateInnerLerpLineString(
                    LeftVertices,
                    RightVertices,
                    PrevBorder->GetPoint(-1),
                    NextBorder->GetPoint(-1),
                    PrevBorder,
                    NextBorder,
                    Rate);

                Right = RnNew<URnWay>(Line, false, true);
            }

            auto L = RnNew<URnWay>(Left->LineString, Left->IsReversed, false);
            auto R = RnNew<URnWay>(Right->LineString, Right->IsReversed, true);
            auto NewLane = RnNew<URnLane>(L, R, PrevBorders[N], NextBorders[N]);
            Lanes.Add(NewLane);
            Left = Right;
        }

        Result.Add(Road, Lanes);
    }

    return Result;
}

