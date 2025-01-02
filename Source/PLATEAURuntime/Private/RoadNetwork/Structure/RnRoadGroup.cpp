#include "RoadNetwork/Structure/RnRoadGroup.h"

#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "RoadNetwork/Structure/RnLineString.h"
#include "RoadNetwork/Structure/RnPoint.h"
#include "RoadNetwork/Util/RnEx.h"

RnRoadGroup::RnRoadGroup(RnRef_t<RnIntersection> InPrevIntersection,
                         RnRef_t<RnIntersection> InNextIntersection,
                         const TArray<RnRef_t<RnRoad>>& InRoads)
    : PrevIntersection(InPrevIntersection)
    , NextIntersection(InNextIntersection) {
    Roads = MakeShared<TArray<RnRef_t<RnRoad>>>(InRoads);
    Align();
}

bool RnRoadGroup::IsValid() const
{
    return Roads->Num() > 0 && std::all_of(Roads->begin(), Roads->end(), [](const RnRef_t<RnRoad>& Road) { return Road->IsValid(); });
}

int32 RnRoadGroup::GetLeftLaneCount() const {
    Align();
    if (Roads->Num() == 0) return 0;

    auto Ret = INT_MAX;
    for (auto& Road : *Roads) {
        Ret = FMath::Min(Ret, Road->GetLeftLaneCount());
    }
    return Ret;
}

int32 RnRoadGroup::GetRightLaneCount() const {
    Align();
    if (Roads->Num() == 0) return 0;

    auto Ret = INT_MAX;
    for (auto& Road : *Roads) {
        Ret = FMath::Min(Ret, Road->GetRightLaneCount());
    }
    return Ret;
}

TArray<RnRef_t<RnLane>> RnRoadGroup::GetRightLanes() const {
    Align();
    TArray<RnRef_t<RnLane>> Result;
    for (const auto& Road : *Roads) {
        Result.Append(Road->GetRightLanes());
    }
    return Result;
}

TArray<RnRef_t<RnLane>> RnRoadGroup::GetLeftLanes() const {
    Align();
    TArray<RnRef_t<RnLane>> Result;
    for (const auto& Road : *Roads) {
        Result.Append(Road->GetLeftLanes());
    }
    return Result;
}

TArray<RnRef_t<RnLane>> RnRoadGroup::GetLanes(ERnDir Dir) const {
    switch (Dir) {
    case ERnDir::Left:
        return GetLeftLanes();
    case ERnDir::Right:
        return GetRightLanes();
    default:
        checkf(false, TEXT("Invalid direction in GetLanes"));
        return TArray<RnRef_t<RnLane>>();
    }
}

void RnRoadGroup::GetMedians(TArray<RnRef_t<RnWay>>& OutLeft, TArray<RnRef_t<RnWay>>& OutRight) const {
    OutLeft.Empty(Roads->Num());
    OutRight.Empty(Roads->Num());

    for (const auto& Road : *Roads) {
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

bool RnRoadGroup::HasMedians() const {
    return Roads->ContainsByPredicate([](const RnRef_t<RnRoad>& Road) { return Road->MedianLane != nullptr; });
}

bool RnRoadGroup::Align() const
{
    return const_cast<RnRoadGroup*>(this)->Align();
}

bool RnRoadGroup::CreateMedianOrSkip(float MedianWidth, float MaxMedianLaneRate) {
    if (HasMedian()) return false;

    if (GetLeftLaneCount() == 0 || GetRightLaneCount() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create median without both left and right lanes"));
        return false;
    }

    MedianWidth = FMath::Max(1.0f, MedianWidth);
    float Width = 0.0f;
    for (const auto& Road : *Roads) {
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

bool RnRoadGroup::HasMedian() const {
    return std::all_of(Roads->begin(), Roads->end(), [](const RnRef_t<RnRoad>& Road) { return Road->MedianLane != nullptr; });
}

bool RnRoadGroup::IsAligned() const {
    if (Roads->Num() <= 1) return true;

    for (int32 i = 0; i < Roads->Num(); ++i) {
        if (i < Roads->Num() - 1 && (*Roads)[i]->Next != (*Roads)[i + 1])
            return false;

        if (i > 0 && (*Roads)[i]->Prev != (*Roads)[i - 1])
            return false;
    }

    return true;
}

bool RnRoadGroup::IsDeepAligned() const {
    if (!IsAligned()) return false;
    if (Roads->Num() <= 1) return true;

    for (int32 i = 1; i < Roads->Num(); ++i) {
        auto PrevRoad = (*Roads)[i - 1];
        auto NowRoad = (*Roads)[i];
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

bool RnRoadGroup::Align() {
    if (IsAligned()) return true;

    // Roads.Count <= 1の場合はIsAligned=trueなのでここでは
    // インデックス範囲外チェックはしなくてよい
    // 0番目が逆かどうかチェック
    if ((*Roads)[0]->Next != (*Roads)[1]) {
        (*Roads)[0]->Reverse();
    }

    // 1番目以降が逆かどうかチェック
    for (int32 i = 1; i < Roads->Num(); ++i) {
        if ((*Roads)[i]->Prev != (*Roads)[i - 1]) {
            (*Roads)[i]->Reverse();
        }
    }

    // 境界線の向きもそろえる
    for (const auto& Road : *Roads) {
        Road->AlignLaneBorder();
    }

    if ((*Roads)[0]->Prev != PrevIntersection) {
        Swap(PrevIntersection, NextIntersection);
    }

    return IsDeepAligned();
}

bool RnRoadGroup::MergeRoads() {
    if (!Align()) return false;
    if (Roads->Num() <= 1) return true;

    // マージ先の道路
    auto DstRoad = (*Roads)[0];
    auto DstLanes = DstRoad->GetAllLanesWithMedian();
    auto DstSideWalks = DstRoad->SideWalks;

    for (int32 i = 1; i < Roads->Num(); i++) {
        // マージ元の道路. DstRoadに統合されて消える
        auto SrcRoad = (*Roads)[i];
        auto SrcLanes = SrcRoad->GetAllLanesWithMedian();

        // SideWalksと共通のLineStringがあるとき, レーン側は統合されるけど
        // SideWalksは統合されない場合もある. その時はLineStringを分離する必要があるので
        // 元のLineStringをコピーして持っておく
        auto OriginalDstSideWalks = DstSideWalks;
        TMap<RnRef_t<RnWay>, RnRef_t<RnWay>> Original;
        for (const auto& SideWalk : *DstSideWalks) {
            for (const auto& Way : SideWalk->GetSideWays()) {
                if (Way) {
                    Original.Add(Way, Way->Clone(false));
                }
            }
        }

        // SideWalksと共通のLineStringもあるので2回追加しないように記録しておく
        TSet<RnRef_t<RnLineString>> Visited;
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

        auto SrcSideWalks = SrcRoad->SideWalks;
        TSet<RnRef_t<RnSideWalk>> MergedDstSideWalks;

        for (const auto& SrcSw : *SrcSideWalks) {
            bool Found = false;
            for (const auto& DstSw : *DstSideWalks) {
                auto MergeSideWalk = [&](bool Reverse, const TFunction<void(RnRef_t<RnWay>, RnRef_t<RnWay>)>& Merger) {
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
                    MergeSideWalk(true, [](RnRef_t<RnWay> A, RnRef_t<RnWay> B) { A->AppendFront2LineString(B); });
                    DstSw->SetStartEdgeWay(SrcSw->GetEndEdgeWay());
                }
                // start - endで重なっている場合
                else if (DstSw->GetStartEdgeWay() && SrcSw->GetEndEdgeWay() && DstSw->GetStartEdgeWay()->IsSameLineReference(SrcSw->GetEndEdgeWay())) {
                    MergeSideWalk(false, [](RnRef_t<RnWay> A, RnRef_t<RnWay> B) { A->AppendFront2LineString(B); });
                    DstSw->SetStartEdgeWay(SrcSw->GetStartEdgeWay());
                }
                // end - endで重なっている場合
                else if (DstSw->GetEndEdgeWay() && SrcSw->GetEndEdgeWay() && DstSw->GetEndEdgeWay()->IsSameLineReference(SrcSw->GetEndEdgeWay())) {
                    MergeSideWalk(true, [](RnRef_t<RnWay> A, RnRef_t<RnWay> B) { A->AppendBack2LineString(B); });
                    DstSw->SetEndEdgeWay(SrcSw->GetStartEdgeWay());
                }
                // end - startで重なっている場合
                else if (DstSw->GetEndEdgeWay() && SrcSw->GetStartEdgeWay() && DstSw->GetEndEdgeWay()->IsSameLineReference(SrcSw->GetStartEdgeWay())) {
                    MergeSideWalk(false, [](RnRef_t<RnWay> A, RnRef_t<RnWay> B) { A->AppendBack2LineString(B); });
                    DstSw->SetEndEdgeWay(SrcSw->GetEndEdgeWay());
                }

                if (Found) break;
            }

            // マージできなかった歩道は直接追加
            if (!Found) {
                SrcSw->SetSideWays(SrcSw->GetOutsideWay(), SrcSw->GetInsideWay());
                DstRoad->AddSideWalk(SrcSw);
                DstSideWalks->Add(SrcSw);
            }
        }

        // DstSideWalksの中でマージされなかった(元の形状から変更されない)ものは
        // レーンと共通のLineStringを持っている場合に勝手に形状変わっているかもしれないので明示的に元に戻す
        for (const auto& Sw : *OriginalDstSideWalks) {
            if (!MergedDstSideWalks.Contains(Sw)) {
                Sw->SetSideWays(
                    Sw->GetOutsideWay() ? Original[Sw->GetOutsideWay()] : nullptr,
                    Sw->GetInsideWay() ? Original[Sw->GetInsideWay()] : nullptr);
            }
        }
        DstRoad->AddTargetTrans(*SrcRoad->TargetTrans);
        SrcRoad->DisConnect(true);
    }

    if (NextIntersection) {
        NextIntersection->RemoveEdges([&](const RnRef_t<RnNeighbor>& Edge) { return Edge->Road == (*Roads)[Roads->Num() - 1]; });
        for (const auto& Lane : DstLanes) {
            NextIntersection->AddEdge(DstRoad, DstRoad->GetBorderWay(Lane, ERnLaneBorderType::Next, ERnLaneBorderDir::Left2Right));
        }
    }

    DstRoad->SetPrevNext(PrevIntersection, NextIntersection);

    Roads = MakeShared<TArray<RnRef_t<RnRoad>>>();
    Roads->Add(DstRoad);
    return true;
}

RnRef_t<RnRoadGroup> RnRoadGroup::CreateRoadGroupOrDefault(RnRef_t<RnIntersection> PrevIntersection, RnRef_t<RnIntersection> NextIntersection) {
    if (!PrevIntersection || !NextIntersection) return nullptr;

    for (const auto& Road : PrevIntersection->GetNeighborRoads()) {
        if (auto RoadPtr = Road->CastToRoad()) {
            TArray<RnRef_t<RnRoad>> Roads = { RoadPtr };
            auto Ret = RnNew<RnRoadGroup>(PrevIntersection, NextIntersection, Roads);
            bool HasPrev = Ret->PrevIntersection == PrevIntersection || Ret->NextIntersection == PrevIntersection;
            bool HasNext = Ret->PrevIntersection == NextIntersection || Ret->NextIntersection == NextIntersection;
            if (HasPrev && HasNext) {
                return Ret;
            }
        }
    }

    return nullptr;
}

bool RnRoadGroup::IsSameRoadGroup(RnRef_t<RnRoadGroup> A, RnRef_t<RnRoadGroup> B) {
    if (!A && !B) return false;

    // 同じ交差点を含むか（Next,Prevは問わない）
    bool IsSameIntersection =
        (A->PrevIntersection == B->PrevIntersection && A->NextIntersection == B->NextIntersection) ||
        (A->PrevIntersection == B->NextIntersection && A->NextIntersection == B->PrevIntersection);

    // 同じ道路を含むか
    bool IsContainSameRoads = true;
    for (const auto& Road : *A->Roads) {
        if (!B->Roads->Contains(Road)) {
            IsContainSameRoads = false;
            break;
        }
    }

    return IsSameIntersection && IsContainSameRoads;
}

void RnRoadGroup::AdjustBorder() {
    Align();

    auto Adjust = [](RnRef_t<RnRoad> Road, ERnLaneBorderType BorderType, RnRef_t<RnIntersection> Inter) {
        if (!Inter) return;

        FLineSegment3D BorderLeft2Right;
        if (!Road->TryGetAdjustBorderSegment(BorderType, BorderLeft2Right))
            return;

        auto LeftWay = Road->GetMergedSideWay(ERnDir::Left);
        auto RightWay = Road->GetMergedSideWay(ERnDir::Right);

        auto DuplicatePoints = [&Inter,&Road](RnRef_t<RnPoint> P, const FVector& CheckVertex) {
            if ((P->Vertex - CheckVertex).SizeSquared() < 1e-6f)
                return;

            for (const auto& Edge : *(Inter->Edges)) {
                if (Edge->Road != Road && Edge->Border->LineString->Contains(P)) {
                    int32 I = Edge->Border->LineString->Points->IndexOfByKey(P);
                    if (I == 0) {
                        Edge->Border->LineString->Points->Insert(RnNew<RnPoint>(P->Vertex), 1);
                    }
                    else if (I == Edge->Border->LineString->Points->Num() - 1) {
                        Edge->Border->LineString->Points->Insert(RnNew<RnPoint>(P->Vertex), Edge->Border->LineString->Points->Num() - 1);
                    }
                }
            }
            };

        int32 LastPointIndex = BorderType == ERnLaneBorderType::Prev ? 0 : -1;
        DuplicatePoints(LeftWay->GetPoint(LastPointIndex), BorderLeft2Right.GetStart());
        DuplicatePoints(RightWay->GetPoint(LastPointIndex), BorderLeft2Right.GetEnd());

        auto AdjustWay = [&](RnRef_t<RnLane> Lane, RnRef_t<RnWay> Way) {
            if (Lane->IsReverse)
                Way = Way->ReversedWay();
            auto P = Way->GetPoint(LastPointIndex);
            auto N = BorderLeft2Right.GetNearestPoint(P->Vertex);
            P->Vertex = N;
            };

        auto Lanes = Road->GetAllLanesWithMedian();
        TArray<RnRef_t<RnWay>> NewBorders;
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

            auto Points = MakeShared<TArray<RnRef_t<RnPoint>>>();
            Points->Add(L->GetPoint(LastPointIndex));
            Points->Add(R->GetPoint(LastPointIndex));

            auto Ls = RnNew<RnLineString>();
            Ls->Points = Points;
            NewBorders.Add(RnNew<RnWay>(Ls));
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

    Adjust((*Roads)[Roads->Num() - 1], ERnLaneBorderType::Next, NextIntersection);
    Adjust((*Roads)[0], ERnLaneBorderType::Prev, PrevIntersection);
}
void RnRoadGroup::SetLaneCountImpl(int32 Count, ERnDir Dir, bool RebuildTrack)
{
    if (IsValid() == false)
        return;

    // 既に指定の数になっている場合は何もしない
    if (std::all_of(Roads->begin(), Roads->end(), [Count, Dir](const RnRef_t<RnRoad> R) {
        return  R->GetLanes(Dir).Num() == Count;
        })) {
        return;
    }

    const auto AfterLanes = SplitLane(Count, Dir);
    if (!AfterLanes)
        return;

    for(auto I = 0; I < Roads->Num(); ++I)
    {
        auto Road = (*Roads)[I];
        auto Lanes =(*AfterLanes)[Road];

        auto BeforeLanes = Road->GetLanes(Dir);
        if (I == Roads->Num() - 1 && NextIntersection) 
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

        (*Roads)[I]->ReplaceLanes(Lanes, Dir);
    }
}

void RnRoadGroup::SetLaneCountWithoutMedian(int32 LeftCount, int32 RightCount, bool RebuildTrack) {
    if (std::all_of(Roads->begin(), Roads->end(), [LeftCount, RightCount](const RnRef_t<RnRoad>& Road) {
        return Road->GetLeftLaneCount() == LeftCount && Road->GetRightLaneCount() == RightCount;
        })) {
        return;
    }

    Align();

    auto Num = LeftCount + RightCount;
    auto AfterLanes = SplitLane(Num, std::nullopt);
    if (!AfterLanes) return;

    TArray<RnRef_t<RnLineString>> NewNextBorders;
    TArray<RnRef_t<RnLineString>> NewPrevBorders;

    for (int32 i = 0; i < Roads->Num(); ++i) {
        auto Road = (*Roads)[i];
        auto Lanes = (*AfterLanes)[Road];

        if (i == Roads->Num() - 1) {
            NextIntersection->ReplaceEdges(Road, RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnWay>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->NextBorder; }));
            NewNextBorders.Append(RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnLineString>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->NextBorder->LineString; }));
        }

        if (i == 0) {
            PrevIntersection->ReplaceEdges(Road, RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnWay>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->PrevBorder; }));
            NewPrevBorders.Append(RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnLineString>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->PrevBorder->LineString; }));
        }

        for (int32 j = LeftCount; j < Lanes.Num(); ++j) {
            Lanes[j]->Reverse();
        }

        Road->ReplaceLanes(Lanes);
    }

    // 中央分離帯を削除する
    for (const auto& Road : *Roads) {
        Road->SetMedianLane(nullptr);
    }
}

void RnRoadGroup::SetLaneCount(int32 LeftCount, int32 RightCount, bool RebuildTrack) {
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

void RnRoadGroup::SetLaneCountWithMedian(int32 LeftCount, int32 RightCount, float MedianWidthRate) {
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

    if (!AfterLanes) return;

    TArray<RnRef_t<RnLineString>> NewNextBorders;
    TArray<RnRef_t<RnLineString>> NewPrevBorders;

    for (int32 i = 0; i < Roads->Num(); ++i) {
        auto Road = (*Roads)[i];
        auto Lanes = (*AfterLanes)[Road];

        if (i == Roads->Num() - 1) 
        {
            NextIntersection->ReplaceEdges(Road, RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnWay>>( Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->NextBorder; }));
            NewNextBorders.Append( RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnLineString>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->NextBorder->LineString; }));
        }

        if (i == 0) {
            PrevIntersection->ReplaceEdges(Road, RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnWay>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->PrevBorder; }));
            NewPrevBorders.Append(RnEx::Map<RnRef_t<RnLane>, RnRef_t<RnLineString>>(Lanes, [](const RnRef_t<RnLane>& Lane) { return Lane->PrevBorder->LineString; }));
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

void RnRoadGroup::SetLeftLaneCount(int32 Count, bool RebuildTrack) {
    if (GetLeftLaneCount() == Count) return;

    if (GetLeftLaneCount() == 0 || Count == 0) {
        SetLaneCountWithoutMedian(Count, GetRightLaneCount(), RebuildTrack);
    }
    else {
        SetLaneCountImpl(Count, ERnDir::Left, RebuildTrack);
    }
}

void RnRoadGroup::SetRightLaneCount(int32 Count, bool RebuildTrack) {
    if (GetRightLaneCount() == Count) return;

    if (GetRightLaneCount() == 0 || Count == 0) {
        SetLaneCountWithoutMedian(GetLeftLaneCount(), Count, RebuildTrack);
    }
    else {
        SetLaneCountImpl(Count, ERnDir::Right, RebuildTrack);
    }
}

void RnRoadGroup::SetLaneCount(ERnDir Dir, int32 Count, bool RebuildTrack) {
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
TSharedPtr<TMap<RnRef_t<RnRoad>, TArray<RnRef_t<RnLane>>>> RnRoadGroup::SplitLane(
    int32 Num,
    std::optional<ERnDir> Dir,
    TFunction<float(int32)> GetSplitRate) {
    if (Num <= 0) return nullptr;

    auto Result = MakeShared<TMap<RnRef_t<RnRoad>, TArray<RnRef_t<RnLane>>>>();

    auto MergedBorders = RnEx::Map<RnRef_t<RnRoad>, RnRef_t<RnWay>>(*Roads, [Dir](const RnRef_t<RnRoad>& Road) {
        return Road->GetMergedBorder(ERnLaneBorderType::Prev, Dir);
        });
    MergedBorders.Add((*Roads)[Roads->Num() - 1]->GetMergedBorder(ERnLaneBorderType::Next, Dir));

    TArray<TArray<RnRef_t<RnWay>>> BorderWays;
    BorderWays.Reserve(Roads->Num() + 1);

    for (const auto& B : MergedBorders) {
        auto Split = B->Split(Num, false, GetSplitRate);
        BorderWays.Add(Split);
    }

    for (int32 i = 0; i < Roads->Num(); ++i) {
        auto Road = (*Roads)[i];
        auto PrevBorders = BorderWays[i];
        auto NextBorders = BorderWays[i + 1];

        RnRef_t<RnWay> LeftWay, RightWay;
        Road->TryGetMergedSideWay(Dir, LeftWay, RightWay);

        auto LeftVertices = LeftWay->GetVertices().ToArray();
        auto RightVertices = RightWay->GetVertices().ToArray();
        auto Left = LeftWay;
        TArray<RnRef_t<RnLane>> Lanes;
        Lanes.Reserve(Num);
        float Rate = 0.0f;

        for (int32 N = 0; N < Num; ++N) {
            auto Right = RightWay;
            if (N < Num - 1) {
                Rate += GetSplitRate ? GetSplitRate(N) : (1.0f / Num);
                auto PrevBorder = PrevBorders[N];
                auto NextBorder = NextBorders[N];
                auto Line = RnEx::CreateInnerLerpLineString(
                    LeftVertices,
                    RightVertices,
                    PrevBorder->GetPoint(-1),
                    NextBorder->GetPoint(-1),
                    PrevBorder,
                    NextBorder,
                    Rate);

                Right = RnNew<RnWay>(Line, false, true);
            }

            auto L = RnNew<RnWay>(Left->LineString, Left->IsReversed, false);
            auto R = RnNew<RnWay>(Right->LineString, Right->IsReversed, true);
            auto NewLane = RnNew<RnLane>(L, R, PrevBorders[N], NextBorders[N]);
            Lanes.Add(NewLane);
            Left = Right;
        }

        Result->Add(Road, Lanes);
    }

    return Result;
}

