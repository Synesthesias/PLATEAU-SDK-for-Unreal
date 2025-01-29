#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnRoadGroup.h"
#include "RoadNetwork/Structure/RnWay.h"

const FString& URnModel::GetFactoryVersion() const
{
    return FactoryVersion;
}

void URnModel::SetFactoryVersion(const FString& InFactoryVersion)
{
    this->FactoryVersion = InFactoryVersion;
}

URnModel::URnModel() {
}

void URnModel::AddRoadBase(const TRnRef_T<URnRoadBase>& RoadBase)
{
    if (!RoadBase) 
        return;
    if (auto Road = RoadBase->CastToRoad()) {
        AddRoad(Road);
    }
    else if (auto Intersection = RoadBase->CastToIntersection()) {
        AddIntersection(Intersection);
    }
}

void URnModel::AddRoad(const TRnRef_T<URnRoad>& Road) {
    if (!Road) return;
    Road->SetParentModel(TRnRef_T<URnModel>(this));
    Roads.AddUnique(Road);
}

void URnModel::RemoveRoad(const TRnRef_T<URnRoad>& Road) {
    if (!Road) return;
    Road->SetParentModel(nullptr);
    Roads.Remove(Road);
}

void URnModel::AddIntersection(const TRnRef_T<URnIntersection>& Intersection) {
    if (!Intersection) return;
    Intersection->SetParentModel(TRnRef_T<URnModel>(this));
    Intersections.AddUnique(Intersection);
}

void URnModel::RemoveIntersection(const TRnRef_T<URnIntersection>& Intersection) {
    if (!Intersection) return;
    Intersection->SetParentModel(nullptr);
    Intersections.Remove(Intersection);
}

void URnModel::AddSideWalk(const TRnRef_T<URnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalks.AddUnique(SideWalk);
}

void URnModel::RemoveSideWalk(const TRnRef_T<URnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalks.Remove(SideWalk);
}

const TArray<TRnRef_T<URnRoad>>& URnModel::GetRoads() const {
    return Roads;
}

const TArray<TRnRef_T<URnIntersection>>& URnModel::GetIntersections() const {
    return Intersections;
}

const TArray<TRnRef_T<URnSideWalk>>& URnModel::GetSideWalks() const {
    return SideWalks;
}

TRnRef_T<URnRoad> URnModel::GetRoadBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& Road : Roads) {
        if (Road->GetTargetTrans().Contains(TargetTran)) {
            return Road;
        }
    }
    return nullptr;
}

TRnRef_T<URnIntersection> URnModel::GetIntersectionBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& Intersection : Intersections) {
        if (Intersection->GetTargetTrans().Contains(TargetTran)) {
            return Intersection;
        }
    }
    return nullptr;
}

TRnRef_T<URnSideWalk> URnModel::GetSideWalkBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& SideWalk : SideWalks) {
        if (SideWalk->GetParentRoad() && SideWalk->GetParentRoad()->GetTargetTrans().Contains(TargetTran)) {
            return SideWalk;
        }
    }
    return nullptr;
}

TRnRef_T<URnRoadBase> URnModel::GetRoadBaseBy(UPLATEAUCityObjectGroup* TargetTran) const {
    auto Road = GetRoadBy(TargetTran);
    if (Road) return Road;
    return GetIntersectionBy(TargetTran);
}

TArray<TRnRef_T<URnRoadBase>> URnModel::GetNeighborRoadBases(const TRnRef_T<URnRoadBase>& RoadBase) const {
    if (!RoadBase) return TArray<TRnRef_T<URnRoadBase>>();
    return RoadBase->GetNeighborRoads();
}

TArray<TRnRef_T<URnRoad>> URnModel::GetNeighborRoads(const TRnRef_T<URnRoadBase>& RoadBase) const {
    TArray<TRnRef_T<URnRoad>> Result;
    for (const auto& Neighbor : GetNeighborRoadBases(RoadBase)) {
        if (auto R = Neighbor->CastToRoad()) {
            Result.Add(R);
        }
    }
    return Result;
}

TArray<TRnRef_T<URnIntersection>> URnModel::GetNeighborIntersections(const TRnRef_T<URnRoadBase>& RoadBase) const {
    TArray<TRnRef_T<URnIntersection>> Result;
    for (const auto& Neighbor : GetNeighborRoadBases(RoadBase)) {
        if (auto Intersection = Neighbor->CastToIntersection()) {
            Result.Add(Intersection);
        }
    }
    return Result;
}

TArray<TRnRef_T<URnSideWalk>> URnModel::GetNeighborSideWalks(const TRnRef_T<URnRoadBase>& RoadBase) const {
    if (!RoadBase )
        return TArray<TRnRef_T<URnSideWalk>>();
    return RoadBase->GetSideWalks();
}

void URnModel::CalibrateIntersectionBorder(const FRnModelCalibrateIntersectionBorderOption& Option) {
    for (const auto& Intersection : Intersections) {
        for (const auto& Edge : Intersection->GetEdges()) {
            if (!Edge->GetRoad()) continue;

            auto Road = Edge->GetRoad()->CastToRoad();
            if (!Road) continue;

            // 道路の長さを取得
            float RoadLength = 0.0f;
            TRnRef_T<URnWay> LeftWay, RightWay;
            if (Road->TryGetMergedSideWay(NullOpt, LeftWay, RightWay)) {
                RoadLength = (LeftWay->CalcLength() + RightWay->CalcLength()) * 0.5f;
            }

            // 道路の長さが必要な長さより短い場合は調整量を減らす
            float Offset = Option.MaxOffsetMeter;
            if (RoadLength < Option.NeedRoadLengthMeter) {
                Offset *= RoadLength / Option.NeedRoadLengthMeter;
            }

            // 境界線を移動
            for (const auto& Lane : Edge->GetConnectedLanes()) {
                auto Border = Edge->GetBorder();
                if (!Border) continue;

                // 境界線の方向を取得
                FVector Dir = Border->GetEdgeNormal(0);
                Dir.Z = 0.0f;
                Dir.Normalize();

                // 境界線を移動
                for (auto& Point : Border->LineString->GetPoints()) {
                    Point->Vertex += Dir * Offset;
                }
            }
        }
    }
}

TRnRef_T<URnModel> URnModel::Create() {
    return RnNew<URnModel>();
}

TArray<TRnRef_T<URnRoadBase>> URnModel::GetConnectedRoadBases(const TRnRef_T<URnRoadBase>& RoadBase) const {
    return GetNeighborRoadBases(RoadBase);
}

TArray<TRnRef_T<URnRoad>> URnModel::GetConnectedRoads(const TRnRef_T<URnRoadBase>& RoadBase) const {
    return GetNeighborRoads(RoadBase);
}

TArray<TRnRef_T<URnIntersection>> URnModel::GetConnectedIntersections(const TRnRef_T<URnRoadBase>& RoadBase) const {
    return GetNeighborIntersections(RoadBase);
}

TArray<TRnRef_T<URnSideWalk>> URnModel::GetConnectedSideWalks(const TRnRef_T<URnRoadBase>& RoadBase) const {
    return GetNeighborSideWalks(RoadBase);
}

TArray<TRnRef_T<URnRoadBase>> URnModel::GetConnectedRoadBasesRecursive(const TRnRef_T<URnRoadBase>& RoadBase) const {
    TArray<TRnRef_T<URnRoadBase>> Result;
    TSet<TRnRef_T<URnRoadBase>> Visited;

    if (!RoadBase) return Result;

    TArray<TRnRef_T<URnRoadBase>> Stack;
    Stack.Push(RoadBase);
    Visited.Add(RoadBase);

    while (Stack.Num() > 0) {
        auto Current = Stack.Pop();
        Result.Add(Current);

        for (const auto& Connected : GetConnectedRoadBases(Current)) {
            if (!Visited.Contains(Connected)) {
                Stack.Push(Connected);
                Visited.Add(Connected);
            }
        }
    }

    return Result;
}

void URnModel::MergeRoadGroup()
{
    TSet<TRnRef_T<URnRoad>> visitedRoads;
    auto CopiedRoads = Roads;
    for(auto& road : CopiedRoads)
    {
        if (visitedRoads.Contains(road))
            continue;

        auto roadGroup = URnRoadGroup::CreateRoadGroupOrDefault(road);
        for(auto Road : roadGroup->Roads)
        {
            visitedRoads.Add(Road);
        }
        roadGroup->MergeRoads();
    }
}

void URnModel::SplitLaneByWidth(float RoadWidthMeter, bool rebuildTrack, TArray<FString>& failedRoads)
{
    failedRoads.Reset();
    TSet<TRnRef_T<URnRoad>> visitedRoads;
    // メートルをユニットに変換
    const auto RoadWidth = FPLATEAURnDef::Meter2Unit * RoadWidthMeter;
    for(auto&& Road : Roads) 
    {
        if (visitedRoads.Contains(Road))
            continue;

        try {
            auto&& RoadGroup = URnRoadGroup::CreateRoadGroupOrDefault(Road);
            for(auto&& l : RoadGroup->Roads)
                visitedRoads.Add(l);

            RoadGroup->Align();
            if (RoadGroup->IsValid() == false)
                continue;

            if (RoadGroup->Roads.ContainsByPredicate([](TRnRef_T<URnRoad> l) { return l->MainLanes[0]->HasBothBorder() == false; }))
                continue;
            auto&& leftCount = RoadGroup->GetLeftLaneCount();
            auto&& rightCount = RoadGroup->GetRightLaneCount();
            // すでにレーンが分かれている場合、左右で独立して分割を行う
            auto GetWidth = [&](TOptional<EPLATEAURnDir> Dir)
            {
                auto Width = FLT_MAX;
                for (auto&& Road : RoadGroup->Roads) {
                    auto&& WidthSum = 0.f;
                    TArray<TRnRef_T<URnLane>> OutLanes;
                    Road->TryGetLanes(Dir, OutLanes);
                    for (auto&& l : OutLanes)
                        WidthSum += l->CalcWidth();
                    Width = FMath::Min(Width, WidthSum);
                }
                return Width;
            };
            if (leftCount > 0 && rightCount > 0) 
            {
                for(auto&& dir : { EPLATEAURnDir::Left, EPLATEAURnDir::Right }) 
                {
                    auto Width = GetWidth(dir);
                    auto&& Num = (int)(Width / RoadWidth);                   
                    RoadGroup->SetLaneCount(dir, Num, rebuildTrack);
                }
            }
            // 
            else {
                auto&& Width = GetWidth(NullOpt);
                auto&& Num = (int)(Width / RoadWidth);
                if (Num <= 1)
                    continue;

                auto&& LeftLaneCount = (Num + 1) / 2;
                auto&& RightLaneCount = Num - LeftLaneCount;
                RoadGroup->SetLaneCount(LeftLaneCount, RightLaneCount, rebuildTrack);
            }

        }
        catch (std::exception e) {
            failedRoads.Add(Road->GetName());
        }
    }
}

bool URnModel::Check() const
{
    for (auto&& Road : Roads) {
        if (Road->Check() == false)
            return false;
    }
    for (auto&& Intersection : Intersections) {
        if (Intersection->Check() == false)
            return false;
    }
    for (auto&& SideWalk : SideWalks) {
        if (SideWalk->Check() == false)
            return false;
    }
    return true;
}
