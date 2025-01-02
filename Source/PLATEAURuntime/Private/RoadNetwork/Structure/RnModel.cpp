#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnIntersection.h"
#include "RoadNetwork/Structure/RnSideWalk.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "RoadNetwork/Structure/RnWay.h"

RnModel::RnModel() {
    Roads = MakeShared<TArray<RnRef_t<RnRoad>>>();
    Intersections = MakeShared<TArray<RnRef_t<RnIntersection>>>();
    SideWalks = MakeShared<TArray<RnRef_t<RnSideWalk>>>();
}

void RnModel::AddRoad(const RnRef_t<RnRoad>& Road) {
    if (!Road) return;
    Road->ParentModel = RnRef_t<RnModel>(this);
    Roads->AddUnique(Road);
}

void RnModel::RemoveRoad(const RnRef_t<RnRoad>& Road) {
    if (!Road) return;
    Road->ParentModel = nullptr;
    Roads->Remove(Road);
}

void RnModel::AddIntersection(const RnRef_t<RnIntersection>& Intersection) {
    if (!Intersection) return;
    Intersection->ParentModel = RnRef_t<RnModel>(this);
    Intersections->AddUnique(Intersection);
}

void RnModel::RemoveIntersection(const RnRef_t<RnIntersection>& Intersection) {
    if (!Intersection) return;
    Intersection->ParentModel = nullptr;
    Intersections->Remove(Intersection);
}

void RnModel::AddSideWalk(const RnRef_t<RnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalks->AddUnique(SideWalk);
}

void RnModel::RemoveSideWalk(const RnRef_t<RnSideWalk>& SideWalk) {
    if (!SideWalk) return;
    SideWalks->Remove(SideWalk);
}

TArray<RnRef_t<RnRoad>> RnModel::GetRoads() const {
    return *Roads;
}

TArray<RnRef_t<RnIntersection>> RnModel::GetIntersections() const {
    return *Intersections;
}

TArray<RnRef_t<RnSideWalk>> RnModel::GetSideWalks() const {
    return *SideWalks;
}

RnRef_t<RnRoad> RnModel::GetRoadBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& Road : *Roads) {
        if (Road->TargetTrans->Contains(TargetTran)) {
            return Road;
        }
    }
    return nullptr;
}

RnRef_t<RnIntersection> RnModel::GetIntersectionBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& Intersection : *Intersections) {
        if (Intersection->TargetTrans->Contains(TargetTran)) {
            return Intersection;
        }
    }
    return nullptr;
}

RnRef_t<RnSideWalk> RnModel::GetSideWalkBy(UPLATEAUCityObjectGroup* TargetTran) const {
    if (!TargetTran) return nullptr;

    for (const auto& SideWalk : *SideWalks) {
        if (SideWalk->GetParentRoad() && SideWalk->GetParentRoad()->TargetTrans->Contains(TargetTran)) {
            return SideWalk;
        }
    }
    return nullptr;
}

RnRef_t<RnRoadBase> RnModel::GetRoadBaseBy(UPLATEAUCityObjectGroup* TargetTran) const {
    auto Road = GetRoadBy(TargetTran);
    if (Road) return Road;
    return GetIntersectionBy(TargetTran);
}

TArray<RnRef_t<RnRoadBase>> RnModel::GetNeighborRoadBases(const RnRef_t<RnRoadBase>& RoadBase) const {
    if (!RoadBase) return TArray<RnRef_t<RnRoadBase>>();
    return RoadBase->GetNeighborRoads();
}

TArray<RnRef_t<RnRoad>> RnModel::GetNeighborRoads(const RnRef_t<RnRoadBase>& RoadBase) const {
    TArray<RnRef_t<RnRoad>> Result;
    for (const auto& Neighbor : GetNeighborRoadBases(RoadBase)) {
        if (auto R = Neighbor->CastToRoad()) {
            Result.Add(R);
        }
    }
    return Result;
}

TArray<RnRef_t<RnIntersection>> RnModel::GetNeighborIntersections(const RnRef_t<RnRoadBase>& RoadBase) const {
    TArray<RnRef_t<RnIntersection>> Result;
    for (const auto& Neighbor : GetNeighborRoadBases(RoadBase)) {
        if (auto Intersection = Neighbor->CastToIntersection()) {
            Result.Add(Intersection);
        }
    }
    return Result;
}

TArray<RnRef_t<RnSideWalk>> RnModel::GetNeighborSideWalks(const RnRef_t<RnRoadBase>& RoadBase) const {
    if (!RoadBase || !RoadBase->SideWalks)
        return TArray<RnRef_t<RnSideWalk>>();
    return *(RoadBase->SideWalks);
}

void RnModel::CalibrateIntersectionBorder(const CalibrateIntersectionBorderOption& Option) {
    for (const auto& Intersection : *Intersections) {
        for (const auto& Edge : Intersection->GetEdges()) {
            if (!Edge->Road) continue;

            auto Road = Edge->Road->CastToRoad();
            if (!Road) continue;

            // 道路の長さを取得
            float RoadLength = 0.0f;
            RnRef_t<RnWay> LeftWay, RightWay;
            if (Road->TryGetMergedSideWay(std::nullopt, LeftWay, RightWay)) {
                RoadLength = (LeftWay->CalcLength() + RightWay->CalcLength()) * 0.5f;
            }

            // 道路の長さが必要な長さより短い場合は調整量を減らす
            float Offset = Option.MaxOffsetMeter;
            if (RoadLength < Option.NeedRoadLengthMeter) {
                Offset *= RoadLength / Option.NeedRoadLengthMeter;
            }

            // 境界線を移動
            for (const auto& Lane : Edge->GetConnectedLanes()) {
                auto Border = Edge->Border;
                if (!Border) continue;

                // 境界線の方向を取得
                FVector Dir = Border->GetEdgeNormal(0);
                Dir.Z = 0.0f;
                Dir.Normalize();

                // 境界線を移動
                for (auto& Point : *Border->LineString->Points) {
                    Point->Vertex += Dir * Offset;
                }
            }
        }
    }
}

RnRef_t<RnModel> RnModel::Create() {
    return RnNew<RnModel>();
}

TArray<RnRef_t<RnRoadBase>> RnModel::GetConnectedRoadBases(const RnRef_t<RnRoadBase>& RoadBase) const {
    return GetNeighborRoadBases(RoadBase);
}

TArray<RnRef_t<RnRoad>> RnModel::GetConnectedRoads(const RnRef_t<RnRoadBase>& RoadBase) const {
    return GetNeighborRoads(RoadBase);
}

TArray<RnRef_t<RnIntersection>> RnModel::GetConnectedIntersections(const RnRef_t<RnRoadBase>& RoadBase) const {
    return GetNeighborIntersections(RoadBase);
}

TArray<RnRef_t<RnSideWalk>> RnModel::GetConnectedSideWalks(const RnRef_t<RnRoadBase>& RoadBase) const {
    return GetNeighborSideWalks(RoadBase);
}

TArray<RnRef_t<RnRoadBase>> RnModel::GetConnectedRoadBasesRecursive(const RnRef_t<RnRoadBase>& RoadBase) const {
    TArray<RnRef_t<RnRoadBase>> Result;
    TSet<RnRef_t<RnRoadBase>> Visited;

    if (!RoadBase) return Result;

    TArray<RnRef_t<RnRoadBase>> Stack;
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
