#pragma once

#include "CoreMinimal.h"
#include "RnRoadBase.h"
#include "../RnDef.h"

class RnLane;
class RnWay;
class RnIntersection;
class UPLATEAUCityObjectGroup;

class RnRoad : public RnRoadBase {
public:
    using Super = RnRoadBase;
public:
    RnRoad();
    explicit RnRoad(UPLATEAUCityObjectGroup* TargetTran);
    explicit RnRoad(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans);

    // 接続先(nullの場合は接続なし)
    RnRef_t<RnRoadBase> Next;

    // 接続元(nullの場合は接続なし)
    RnRef_t<RnRoadBase> Prev;

    // レーンリスト
    TSharedPtr<TArray<RnRef_t<RnLane>>> MainLanes;

    // 中央分離帯
    RnRef_t<RnLane> MedianLane;

    // 全レーン
    TArray<RnRef_t<RnLane>> GetAllLanes() const;

    // 中央分離帯を含めた全てのレーン
    TArray<RnRef_t<RnLane>> GetAllLanesWithMedian() const;

    // 有効なRoadかどうか
    bool IsValid() const;

    // 全てのレーンに両方向に接続先がある
    bool IsAllBothConnectedLane() const;

    // 全レーンがIsValidかどうか
    bool IsAllLaneValid() const;

    // 左車線/右車線両方あるかどうか
    bool HasBothLane() const;

    // 交差点間に挿入された空Roadかどうか
    bool IsEmptyRoad() const;

    // 指定方向のレーンを取得
    TArray<RnRef_t<RnLane>> GetLanes(ERnDir Dir) const;

    // レーンが左車線かどうか
    bool IsLeftLane(const RnRef_t<RnLane>& Lane) const;

    // レーンが右車線かどうか
    bool IsRightLane(const RnRef_t<RnLane>& Lane) const;

    // レーンの方向を取得
    ERnDir GetLaneDir(const RnRef_t<RnLane>& Lane) const;

    // 境界線情報を取得
    virtual TArray<RnRef_t<RnWay>> GetBorders() const override;

    // 左側のレーンを取得
    TArray<RnRef_t<RnLane>> GetLeftLanes() const;

    // 右側のレーンを取得
    TArray<RnRef_t<RnLane>> GetRightLanes() const;

    // 左側レーン数を取得
    int32 GetLeftLaneCount() const;

    // 右側レーン数を取得
    int32 GetRightLaneCount() const;

    // 中央分離帯の幅を取得
    float GetMedianWidth() const;

    // 中央分離帯を設定
    void SetMedianLane(const RnRef_t<RnLane>& Lane);

    // 指定したレーンが中央分離帯かどうか
    bool IsMedianLane(const RnRef_t<const RnLane>& Lane) const;

    // 隣接するRoadを取得
    virtual TArray<RnRef_t<RnRoadBase>> GetNeighborRoads() const override;


    /// <summary>
    /// #TODO : 左右の隣接情報がないので要修正
    /// laneを追加する. ParentRoad情報も更新する
    /// </summary>
    /// <param name="Lane"></param>
    void AddMainLane(RnRef_t<RnLane> Lane)
    {
        if (MainLanes->Contains(Lane))
            return;
        OnAddLane(Lane);
        MainLanes->Add(Lane);
    }


    // 指定した方向の境界線を取得する
    virtual RnRef_t<RnWay> GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const override;

    // 指定した方向のWayを取得する
    virtual RnRef_t<RnWay> GetMergedSideWay(ERnDir Dir) const override;

    // 指定した方向のWayを取得する
    bool TryGetMergedSideWay(std::optional<ERnDir> Dir, RnRef_t<RnWay>& OutLeftWay, RnRef_t<RnWay>& OutRightWay) const;

    // 指定したLineStringまでの最短距離を取得する
    virtual bool TryGetNearestDistanceToSideWays(const RnRef_t<RnLineString>& LineString, float& OutDistance) const override;

    // 境界線の向きをそろえる
    virtual void AlignLaneBorder() override;

    // 境界線を調整するための線分を取得する
    virtual bool TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const override;

    // 指定したレーンの境界線を取得する
    virtual RnRef_t<RnWay> GetBorderWay(const RnRef_t<RnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const override;

    // レーンを置き換える
    void ReplaceLanes(const TArray<RnRef_t<RnLane>>& NewLanes, ERnDir Dir);
    void ReplaceLanes(const TArray<RnRef_t<RnLane>>& NewLanes);

    // 前後の接続を設定する
    void SetPrevNext(const RnRef_t<RnRoadBase>& PrevRoad, const RnRef_t<RnRoadBase>& NextRoad);

    // 反転する
    void Reverse();

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const override;

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<RnRef_t<RnWay>> GetAllWays() const override;

    // otherをつながりから削除する
    virtual void UnLink(const RnRef_t<RnRoadBase>& Other) override;

    // 自身の接続を切断する
    virtual void DisConnect(bool RemoveFromModel) override;

    // 隣接情報を置き換える
    virtual void ReplaceNeighbor(const RnRef_t<RnRoadBase>& From, const RnRef_t<RnRoadBase>& To) override;

    // 左右のWayを結合したものを取得
    TArray<RnRef_t<RnWay>> GetMergedSideWays() const;

    // RnRoadへキャストする
    virtual RnRef_t<RnRoad> CastToRoad() override
    {
        return RnRef_t<RnRoad>(this);
    }

    // RnIntersectionへキャストする
    virtual RnRef_t<RnIntersection> CastToIntersection() override
    {
        return RnRef_t<RnIntersection>(nullptr);
    }

    // 道路を作成する
    static RnRef_t<RnRoad> Create(UPLATEAUCityObjectGroup* TargetTran = nullptr);
    static RnRef_t<RnRoad> Create(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans);

    // 孤立した道路を作成する
    static RnRef_t<RnRoad> CreateIsolatedRoad(UPLATEAUCityObjectGroup* TargetTran, RnRef_t<RnWay> Way);

    static RnRef_t<RnRoad> CreateOneLaneRoad(UPLATEAUCityObjectGroup* TargetTran, RnRef_t<RnLane> Lane);

private:

    void OnAddLane(RnRef_t<RnLane> lane);
};
