#pragma once

#include "CoreMinimal.h"
#include "RnRoadBase.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "RoadNetwork/Util/PLATEAURnEx.h"
#include "RnIntersection.generated.h"

class URnWay;
class URnPoint;
class UPLATEAUCityObjectGroup;
class URnRoadBase;
class URnWay;
class URnRoad;

UCLASS()
class URnIntersectionEdge : public UObject
{
    GENERATED_BODY()
public:
    URnIntersectionEdge();
    void Init();
    void Init(TObjectPtr<URnRoadBase> InRoad, TObjectPtr<URnWay> InBorder);

    TRnRef_T<URnRoadBase> GetRoad() const { return Road; }

    TRnRef_T<URnWay> GetBorder() const { return Border; }

    void SetRoad(const TRnRef_T<URnRoadBase>& InRoad) { Road = InRoad; }

    void SetBorder(const TRnRef_T<URnWay>& InBorder) { Border = InBorder; }

    // 有効なNeighborかどうか
    bool IsValid() const;

    bool IsBorder() const;

    // 境界線の中点を取得
    FVector GetCenterPoint() const;

    // 中央分離帯の境界線かどうか
    bool IsMedianBorder() const;

    // 接続されているレーンを取得
    TArray<TRnRef_T<URnLane>> GetConnectedLanes() const;

    // 指定した境界線に接続されているレーンを取得
    TRnRef_T<URnLane> GetConnectedLane(const TRnRef_T<URnWay>& BorderWay) const;

private:

    // 接続先の道路
    UPROPERTY()
    URnRoadBase* Road;

    // 境界線
    UPROPERTY()
    URnWay* Border;
};

UCLASS()
class URnIntersection : public URnRoadBase {
    GENERATED_BODY()
public:
    using Super = URnRoadBase;
public:
    URnIntersection();

    void Init(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran);

    void Init(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans);


    // 他の道路との境界線Edge取得
    TArray<TRnRef_T<URnIntersectionEdge>> GetNeighbors() const;

    // 輪郭のEdge取得
    const TArray<TRnRef_T<URnIntersectionEdge>>& GetEdges() const { return Edges; }

    // 有効な交差点かどうか
    bool IsValid() const;

    // 指定したRoadに接続されているEdgeを取得
    TArray<TRnRef_T<URnIntersectionEdge>> GetEdgesBy(const TRnRef_T<URnRoadBase>& Road) const;

    // 指定したRoadに接続されているEdgeを取得
    TRnRef_T<URnIntersectionEdge> GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const;

    // 指定したRoadに接続されているEdgeを取得
    TRnRef_T<URnIntersectionEdge> GetEdgeBy(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnPoint>& Point) const;

    // 指定したRoadに接続されているEdgeを取得
    TArray<TRnRef_T<URnIntersectionEdge>> GetEdgesBy(const TFunction<bool(const TRnRef_T<URnIntersectionEdge>&)>& Predicate) const;

    // Road/Laneに接続しているEdgeを削除
    void RemoveEdge(const TRnRef_T<URnRoad>& Road, const TRnRef_T<URnLane>& Lane);

    // 指定したRoadに接続されているEdgeを削除
    void RemoveEdges(const TRnRef_T<URnRoadBase>& Road);

    // 指定したRoadに接続されているEdgeを削除
    void RemoveEdges(const TFunction<bool(const TRnRef_T<URnIntersectionEdge>&)>& Predicate);

    // 指定したRoadに接続されているEdgeを置き換える
    void ReplaceEdges(const TRnRef_T<URnRoadBase>& Road, const TArray<TRnRef_T<URnWay>>& NewBorders);

    // Edgeを追加
    void AddEdge(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border);

    // 指定したRoadに接続されているかどうか
    bool HasEdge(const TRnRef_T<URnRoadBase>& Road) const;

    // 指定したRoadに接続されているかどうか
    bool HasEdge(const TRnRef_T<URnRoadBase>& Road, const TRnRef_T<URnWay>& Border) const;

    // Edgesの順番を整列する
    // 各Edgeが連結かつ時計回りになるように整列する
    void Align();

    // 隣接するRoadを取得
    virtual TArray<TRnRef_T<URnRoadBase>> GetNeighborRoads() const override;

    // 境界線情報を取得
    virtual TArray<TRnRef_T<URnWay>> GetBorders() const override;

    // otherをつながりから削除する
    virtual void UnLink(const TRnRef_T<URnRoadBase>& Other) override;

    // 自身の接続を切断する
    virtual void DisConnect(bool RemoveFromModel) override;

    // 隣接情報を置き換える
    virtual void ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) override;

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const override;

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<TRnRef_T<URnWay>> GetAllWays() const override;


    // RnRoadへキャストする
    virtual TRnRef_T<URnRoad> CastToRoad() override {
        return TRnRef_T<URnRoad>(nullptr);
    }

    // RnIntersectionへキャストする
    virtual TRnRef_T<URnIntersection> CastToIntersection() override {
        return TRnRef_T<URnIntersection>(this);
    }

    // 交差点を作成する
    static TRnRef_T<URnIntersection> Create(TObjectPtr<UPLATEAUCityObjectGroup> TargetTran = nullptr);
    static TRnRef_T<URnIntersection> Create(const TArray<TObjectPtr<UPLATEAUCityObjectGroup>>& TargetTrans);

    /// <summary>
    /// 輪郭線の法線方向を外側向くように整える
    /// </summary>
    /// <param name="edge"></param>
    static void AlignEdgeNormal(TRnRef_T<URnIntersectionEdge> edge);

private:

    // 交差点の外形情報
    UPROPERTY()
    TArray<URnIntersectionEdge*> Edges;
};

struct FRnIntersectionEx
{
    struct FEdgeGroup : public FPLATEAURnEx::FKeyEdgeGroup<TRnRef_T<URnRoadBase>, TRnRef_T<URnIntersectionEdge>>
    {
    public:
        bool IsBorder() const
        {
            return (bool)Key;
        }

        bool IsValid() const;



        FEdgeGroup* RightSide = nullptr;
        FEdgeGroup* LeftSide = nullptr;
    };

    static TArray<FEdgeGroup> CreateEdgeGroup(TRnRef_T<URnIntersection> Intersection);
};