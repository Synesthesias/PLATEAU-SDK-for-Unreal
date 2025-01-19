#pragma once

#include "CoreMinimal.h"
#include "RnRoadBase.h"
#include "../RnDef.h"
#include "RnRoad.generated.h"

class URnLane;
class URnWay;
class URnIntersection;
class UPLATEAUCityObjectGroup;

UCLASS()
class URnRoad : public URnRoadBase {
    GENERATED_BODY()
public:
    using Super = URnRoadBase;
public:
    URnRoad();
    explicit URnRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran);
    explicit URnRoad(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans);

    void Init(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran);
    void Init(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans);

    // 全レーン
    const TArray<TRnRef_T<URnLane>>& GetAllLanes() const;

    // 中央分離帯を含めた全てのレーン
    TArray<TRnRef_T<URnLane>> GetAllLanesWithMedian() const;

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
    TArray<TRnRef_T<URnLane>> GetLanes(ERnDir Dir) const;

    // レーンが左車線かどうか
    bool IsLeftLane(const TRnRef_T<URnLane>& Lane) const;

    // レーンが右車線かどうか
    bool IsRightLane(const TRnRef_T<URnLane>& Lane) const;

    // レーンの方向を取得
    ERnDir GetLaneDir(const TRnRef_T<URnLane>& Lane) const;

    // 境界線情報を取得
    virtual TArray<TRnRef_T<URnWay>> GetBorders() const override;

    // 左側のレーンを取得
    TArray<TRnRef_T<URnLane>> GetLeftLanes() const;

    // 右側のレーンを取得
    TArray<TRnRef_T<URnLane>> GetRightLanes() const;

    // 左側レーン数を取得
    int32 GetLeftLaneCount() const;

    // 右側レーン数を取得
    int32 GetRightLaneCount() const;

    // 中央分離帯の幅を取得
    float GetMedianWidth() const;

    // 中央分離帯を設定
    void SetMedianLane(const TRnRef_T<URnLane>& Lane);

    // 指定したレーンが中央分離帯かどうか
    bool IsMedianLane(const TRnRef_T<const URnLane>& Lane) const;

    // 隣接するRoadを取得
    virtual TArray<TRnRef_T<URnRoadBase>> GetNeighborRoads() const override;


    /// <summary>
    /// #TODO : 左右の隣接情報がないので要修正
    /// laneを追加する. ParentRoad情報も更新する
    /// </summary>
    /// <param name="Lane"></param>
    void AddMainLane(TRnRef_T<URnLane> Lane);


    // 指定した方向の境界線を取得する
    virtual TRnRef_T<URnWay> GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const override;

    // 指定した方向のWayを取得する
    virtual TRnRef_T<URnWay> GetMergedSideWay(ERnDir Dir) const override;

    // 指定した方向のWayを取得する
    bool TryGetMergedSideWay(std::optional<ERnDir> Dir, TRnRef_T<URnWay>& OutLeftWay, TRnRef_T<URnWay>& OutRightWay) const;

    // 指定したLineStringまでの最短距離を取得する
    virtual bool TryGetNearestDistanceToSideWays(const TRnRef_T<URnLineString>& LineString, float& OutDistance) const override;

    // 境界線の向きをそろえる
    virtual void AlignLaneBorder() override;

    // 境界線を調整するための線分を取得する
    virtual bool TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const override;

    // 指定したレーンの境界線を取得する
    virtual TRnRef_T<URnWay> GetBorderWay(const TRnRef_T<URnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const override;

    // レーンを置き換える
    void ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes, ERnDir Dir);
    void ReplaceLanes(const TArray<TRnRef_T<URnLane>>& NewLanes);

    // 前後の接続を設定する
    void SetPrevNext(const TRnRef_T<URnRoadBase>& PrevRoad, const TRnRef_T<URnRoadBase>& NextRoad);

    // 反転する
    void Reverse();

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const override;

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<TRnRef_T<URnWay>> GetAllWays() const override;

    // otherをつながりから削除する
    virtual void UnLink(const TRnRef_T<URnRoadBase>& Other) override;

    // 自身の接続を切断する
    virtual void DisConnect(bool RemoveFromModel) override;

    // 隣接情報を置き換える
    virtual void ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) override;

    // 左右のWayを結合したものを取得
    TArray<TRnRef_T<URnWay>> GetMergedSideWays() const;

    // RnRoadへキャストする
    virtual TRnRef_T<URnRoad> CastToRoad() override
    {
        return TRnRef_T<URnRoad>(this);
    }

    // RnIntersectionへキャストする
    virtual TRnRef_T<URnIntersection> CastToIntersection() override
    {
        return TRnRef_T<URnIntersection>(nullptr);
    }

    // 道路を作成する
    static TRnRef_T<URnRoad> Create(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran = nullptr);
    static TRnRef_T<URnRoad> Create(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans);

    // 孤立した道路を作成する
    static TRnRef_T<URnRoad> CreateIsolatedRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran, TRnRef_T<URnWay> Way);

    static TRnRef_T<URnRoad> CreateOneLaneRoad(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran, TRnRef_T<URnLane> Lane);

private:

    void OnAddLane(TRnRef_T<URnLane> lane);

public:
    // 接続先(nullの場合は接続なし)
    UPROPERTY()
    TObjectPtr<URnRoadBase> Next;

    // 接続元(nullの場合は接続なし)
    UPROPERTY()
    TObjectPtr<URnRoadBase> Prev;

    // レーンリスト
    UPROPERTY()
    TArray<TObjectPtr<URnLane>> MainLanes;

    // 中央分離帯
    UPROPERTY()
    TObjectPtr<URnLane> MedianLane;

};
