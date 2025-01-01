#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"

// Forward declarations
class RnRoadBase;
class RnWay;
class RnPoint;

class RnSideWalk {
public:
    RnSideWalk();

    // 自分が所属するRoadNetworkModel
    RnRef_t<RnRoadBase> GetParentRoad() const { return ParentRoad; }

    // 道路と反対側のWay
    RnRef_t<RnWay> GetOutsideWay() const { return OutsideWay; }

    // 道路と同じ側のWay
    RnRef_t<RnWay> GetInsideWay() const { return InsideWay; }

    // outsideWayとinsideWayの始点を繋ぐWay
    RnRef_t<RnWay> GetStartEdgeWay() const { return StartEdgeWay; }

    // outsideWayとinsideWayの終点を繋ぐWay
    RnRef_t<RnWay> GetEndEdgeWay() const { return EndEdgeWay; }

    ERnSideWalkLaneType GetLaneType() const { return LaneType; }
    void SetLaneType(ERnSideWalkLaneType Type) { LaneType = Type; }

    // 左右のWay(OutsideWay, InsideWay)を列挙
    TArray<RnRef_t<RnWay>> GetSideWays() const;

    // 開始/終了の境界線のWay(StartEdgeWay, EndEdgeWay)を列挙
    TArray<RnRef_t<RnWay>> GetEdgeWays() const;

    // 全てのWay(nullは含まない)
    TArray<RnRef_t<RnWay>> GetAllWays() const;

    // Inside/OutsideのWayが両方ともValidかどうか
    bool IsValid() const;

    ERnSideWalkWayTypeMask GetValidWayTypeMask() const;

    // 強制的に親を変更する. 構造壊れるので扱い注意
    void SetParent(const RnRef_t<RnRoadBase>& Parent);

    // 親からのリンク解除
    void UnLinkFromParent();

    // 左右のWayを再設定
    void SetSideWays(const RnRef_t<RnWay>& OutsideWay, const RnRef_t<RnWay>& InsideWay);

    // 境界のWayを再設定
    void SetEdgeWays(const RnRef_t<RnWay>& StartWay, const RnRef_t<RnWay>& EndWay);
    void SetStartEdgeWay(const RnRef_t<RnWay>& StartWay);
    void SetEndEdgeWay(const RnRef_t<RnWay>& EndWay);

    // レーンタイプを入れ替え
    void ReverseLaneType();

    // InSideWay/OutSideWayの方向を合わせる
    void TryAlign();

    // 歩道作成
    static RnRef_t<RnSideWalk> Create(
        const RnRef_t<RnRoadBase>& Parent,
        const RnRef_t<RnWay>& OutsideWay,
        const RnRef_t<RnWay>& InsideWay,
        const RnRef_t<RnWay>& StartEdgeWay,
        const RnRef_t<RnWay>& EndEdgeWay,
        ERnSideWalkLaneType LaneType = ERnSideWalkLaneType::Undefined);

    // 代表点を取得
    FVector GetCentralVertex() const;

    // 隣接判定
    bool IsNeighboring(const RnRef_t<RnSideWalk>& Other) const;

    // 近接スコア計算
    float CalcRoadProximityScore(const RnRef_t<RnRoadBase>& Other) const;

private:
    RnRef_t<RnRoadBase> ParentRoad;
    RnRef_t<RnWay> OutsideWay;
    RnRef_t<RnWay> InsideWay;
    RnRef_t<RnWay> StartEdgeWay;
    RnRef_t<RnWay> EndEdgeWay;
    ERnSideWalkLaneType LaneType;
};
