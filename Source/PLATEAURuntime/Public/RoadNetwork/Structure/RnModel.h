#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"

class RnRoad;
class RnIntersection;
class RnSideWalk;
class RnLane;
class RnNeighbor;
class RnLineString;
class RnWay;
class UPLATEAUCityObjectGroup;
class RnRoadBase;

class RnModel
{
public:

    struct CalibrateIntersectionBorderOption {


        // 交差点の停止線を道路側に移動させる量
        float MaxOffsetMeter = 5.0f;

        // 道路の長さがこれ以下にならないように交差点の移動量を減らす
        float NeedRoadLengthMeter = 23.0f;
    };

public:
    static constexpr float Epsilon = SMALL_NUMBER;
    static constexpr EAxisPlane Plane = EAxisPlane::Xz;

    // 自動生成で作成されたときのバージョン
    FString FactoryVersion;

    // 道路リスト
    TSharedPtr<TArray<RnRef_t<RnRoad>>> Roads;

    // 交差点リスト
    TSharedPtr<TArray<RnRef_t<RnIntersection>>> Intersections;

    // 歩道リスト
    TSharedPtr<TArray<RnRef_t<RnSideWalk>>> SideWalks;

    RnModel();

    // 道路を追加
    void AddRoadBase(const RnRef_t<RnRoadBase>& RoadBase);

    // 道路を追加
    void AddRoad(const RnRef_t<RnRoad>& Road);

    // 道路を削除
    void RemoveRoad(const RnRef_t<RnRoad>& Road);

    // 交差点を追加
    void AddIntersection(const RnRef_t<RnIntersection>& Intersection);

    // 交差点を削除
    void RemoveIntersection(const RnRef_t<RnIntersection>& Intersection);

    // 歩道を追加
    void AddSideWalk(const RnRef_t<RnSideWalk>& SideWalk);

    // 歩道を削除
    void RemoveSideWalk(const RnRef_t<RnSideWalk>& SideWalk);

    // 道路を取得
    TArray<RnRef_t<RnRoad>> GetRoads() const;

    // 交差点を取得
    TArray<RnRef_t<RnIntersection>> GetIntersections() const;

    // 歩道を取得
    TArray<RnRef_t<RnSideWalk>> GetSideWalks() const;

    // 指定したCityObjectGroupを含む道路を取得
    RnRef_t<RnRoad> GetRoadBy(UPLATEAUCityObjectGroup* TargetTran) const;

    // 指定したCityObjectGroupを含む交差点を取得
    RnRef_t<RnIntersection> GetIntersectionBy(UPLATEAUCityObjectGroup* TargetTran) const;

    // 指定したCityObjectGroupを含む歩道を取得
    RnRef_t<RnSideWalk> GetSideWalkBy(UPLATEAUCityObjectGroup* TargetTran) const;

    // 指定したCityObjectGroupを含む道路/交差点を取得
    RnRef_t<RnRoadBase> GetRoadBaseBy(UPLATEAUCityObjectGroup* TargetTran) const;

    // 指定した道路/交差点に接続されている道路/交差点を取得
    TArray<RnRef_t<RnRoadBase>> GetNeighborRoadBases(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定した道路/交差点に接続されている道路を取得
    TArray<RnRef_t<RnRoad>> GetNeighborRoads(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定した道路/交差点に接続されている交差点を取得
    TArray<RnRef_t<RnIntersection>> GetNeighborIntersections(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定した道路/交差点に接続されている歩道を取得
    TArray<RnRef_t<RnSideWalk>> GetNeighborSideWalks(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 交差点の境界線を調整する
    void CalibrateIntersectionBorder(const CalibrateIntersectionBorderOption& Option);

    // 道路ネットワークを作成する
    static RnRef_t<RnModel> Create();

    // 指定したRoadBaseに接続されている道路/交差点を取得
    TArray<RnRef_t<RnRoadBase>> GetConnectedRoadBases(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定したRoadBaseに接続されている道路を取得
    TArray<RnRef_t<RnRoad>> GetConnectedRoads(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定したRoadBaseに接続されている交差点を取得
    TArray<RnRef_t<RnIntersection>> GetConnectedIntersections(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定したRoadBaseに接続されている歩道を取得
    TArray<RnRef_t<RnSideWalk>> GetConnectedSideWalks(const RnRef_t<RnRoadBase>& RoadBase) const;

    // 指定したRoadBaseに接続されているRoadBaseを取得
    TArray<RnRef_t<RnRoadBase>> GetConnectedRoadBasesRecursive(const RnRef_t<RnRoadBase>& RoadBase) const;


};
