#pragma once

#include "CoreMinimal.h"
#include "RnRoadBase.h"
#include "../RnDef.h"

// Forward declarations
class RnNeighbor;
class RnWay;
class RnPoint;
class UPLATEAUCityObjectGroup;
class UTrafficSignalLightController;
class RnRoadBase;
class RnWay;

class RnNeighbor {
public:
    RnNeighbor();

    // 接続先の道路
    RnRef_t<RnRoadBase> Road;

    // 境界線
    RnRef_t<RnWay> Border;

    // 有効なNeighborかどうか
    bool IsValid() const;

    bool IsBorder() const;

    // 境界線の中点を取得
    FVector GetCenterPoint() const;

    // 中央分離帯の境界線かどうか
    bool IsMedianBorder() const;

    // 接続されているレーンを取得
    TArray<RnRef_t<RnLane>> GetConnectedLanes() const;

    // 指定した境界線に接続されているレーンを取得
    RnRef_t<RnLane> GetConnectedLane(const RnRef_t<RnWay>& BorderWay) const;

};

class RnIntersection : public RnRoadBase {
public:
    using Super = RnRoadBase;
public:
    RnIntersection();
    explicit RnIntersection(UPLATEAUCityObjectGroup* TargetTran);
    explicit RnIntersection(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans);

    // 交差点の外形情報
    TSharedPtr<TArray<RnRef_t<RnNeighbor>>> Edges;

    // 信号制御器
    UPROPERTY()
    UTrafficSignalLightController* SignalController;

    // 道路と道路の間に入れる空交差点かどうかの判定
    bool bIsEmptyIntersection;

    // 他の道路との境界線Edge取得
    TArray<RnRef_t<RnNeighbor>> GetNeighbors() const;

    // 輪郭のEdge取得
    const TArray<RnRef_t<RnNeighbor>>& GetEdges() const { return *Edges; }

    // 有効な交差点かどうか
    bool IsValid() const;

    // 指定したRoadに接続されているEdgeを取得
    TArray<RnRef_t<RnNeighbor>> GetEdgesBy(const RnRef_t<RnRoadBase>& Road) const;

    // 指定したRoadに接続されているEdgeを取得
    RnRef_t<RnNeighbor> GetEdgeBy(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border) const;

    // 指定したRoadに接続されているEdgeを取得
    RnRef_t<RnNeighbor> GetEdgeBy(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnPoint>& Point) const;

    // 指定したRoadに接続されているEdgeを取得
    TArray<RnRef_t<RnNeighbor>> GetEdgesBy(const TFunction<bool(const RnRef_t<RnNeighbor>&)>& Predicate) const;

    // 指定したRoadに接続されているEdgeを削除
    void RemoveEdges(const RnRef_t<RnRoadBase>& Road);

    // 指定したRoadに接続されているEdgeを削除
    void RemoveEdges(const TFunction<bool(const RnRef_t<RnNeighbor>&)>& Predicate);

    // 指定したRoadに接続されているEdgeを置き換える
    void ReplaceEdges(const RnRef_t<RnRoadBase>& Road, const TArray<RnRef_t<RnWay>>& NewBorders);

    // Edgeを追加
    void AddEdge(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border);

    // 指定したRoadに接続されているかどうか
    bool HasEdge(const RnRef_t<RnRoadBase>& Road) const;

    // 指定したRoadに接続されているかどうか
    bool HasEdge(const RnRef_t<RnRoadBase>& Road, const RnRef_t<RnWay>& Border) const;
public:
    // 隣接するRoadを取得
    virtual TArray<RnRef_t<RnRoadBase>> GetNeighborRoads() const override;

    // 境界線情報を取得
    virtual TArray<RnRef_t<RnWay>> GetBorders() const override;

    // otherをつながりから削除する
    virtual void UnLink(const RnRef_t<RnRoadBase>& Other) override;

    // 自身の接続を切断する
    virtual void DisConnect(bool RemoveFromModel) override;

    // 隣接情報を置き換える
    virtual void ReplaceNeighbor(const RnRef_t<RnRoadBase>& From, const RnRef_t<RnRoadBase>& To) override;

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const override;

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<RnRef_t<RnWay>> GetAllWays() const override;

    // 交差点を作成する
    static RnRef_t<RnIntersection> Create(UPLATEAUCityObjectGroup* TargetTran = nullptr);
    static RnRef_t<RnIntersection> Create(const TArray<UPLATEAUCityObjectGroup*>& TargetTrans);
private:
    // トラックを生成する
    void BuildTracksImpl(const TSet<RnRef_t<RnLineString>>& Borders);

    // 指定したEdgeから指定したEdgeへのトラックを生成する
    void BuildTrack(const RnRef_t<RnNeighbor>& From, const RnRef_t<RnNeighbor>& To);

    // 指定したEdgeから指定したEdgeへのトラックを生成する
    void BuildTrack(const RnRef_t<RnNeighbor>& From, const RnRef_t<RnNeighbor>& To, const TArray<FVector>& Points);
};
