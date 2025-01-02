#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/GeoGraph/LineSegment3D.h"

class RnModel;
class RnSideWalk;
class RnBorder;
class RnWay;
class RnLineString;
class RnLane;
class RnRoad;
class RnIntersection;
class RnRoadBase
{
public:
    RnRoadBase();
    virtual ~RnRoadBase() = default;

    // 自分が所属するRoadNetworkModel
    RnRef_t<RnModel> ParentModel;

    // これに紐づくtranオブジェクトリスト(統合なので複数存在する場合がある)
    TSharedPtr<TArray<UPLATEAUCityObjectGroup*>> TargetTrans;

    // 歩道情報
    TSharedPtr<TArray<RnRef_t<RnSideWalk>>> SideWalks;

    // 歩道sideWalkを追加する
    // sideWalkの親情報も書き換える
    void AddSideWalk(const RnRef_t<RnSideWalk>& SideWalk);

    // 歩道sideWalkを削除する
    // sideWalkの親情報は変更しない
    void RemoveSideWalk(const RnRef_t<RnSideWalk>& SideWalk);

    // 境界線情報を取得
    virtual TArray<RnRef_t<RnWay>> GetBorders() const { return TArray<RnRef_t<RnWay>>(); }

    // 隣接するRoadを取得
    virtual TArray<RnRef_t<RnRoadBase>> GetNeighborRoads() const { return TArray<RnRef_t<RnRoadBase>>(); }

    // 対象のTargetTranを追加
    void AddTargetTran(UPLATEAUCityObjectGroup* TargetTran);
    void AddTargetTrans(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans);

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<RnRef_t<RnWay>> GetAllWays() const;

    // otherをつながりから削除する. other側の接続は消えない
    virtual void UnLink(const RnRef_t<RnRoadBase>& Other) {}

    // 自身の接続を切断する
    // removeFromModel=trueの場合、RnModelからも削除する
    virtual void DisConnect(bool RemoveFromModel);

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const { return FVector::ZeroVector; }

    virtual void ReplaceNeighbor(const RnRef_t<RnRoadBase>& From, const RnRef_t<RnRoadBase>& To) {}

    // 相互に接続を解除する
    void UnLinkEachOther(const RnRef_t<RnRoadBase>& Other);

    // デバッグ表示用. TargetTransの名前を取得
    FString GetTargetTransName() const;

    // selfのすべてのLineStringを取得
    TSet<RnRef_t<RnLineString>> GetAllLineStringsDistinct() const;

    // 歩道を取得
    const TSharedPtr<TArray<RnRef_t<RnSideWalk>>>& GetSideWalks() const { return SideWalks; }

    // 指定した方向の境界線を取得する
    virtual RnRef_t<RnWay> GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const { return nullptr; }

    // 指定した方向のWayを取得する
    virtual RnRef_t<RnWay> GetMergedSideWay(ERnDir Dir) const { return nullptr; }

    // 指定した方向のWayを取得する
    virtual bool TryGetMergedSideWay(ERnDir Dir, RnRef_t<RnWay>& OutLeftWay, RnRef_t<RnWay>& OutRightWay) const { return false; }

    // 指定したLineStringまでの最短距離を取得する
    virtual bool TryGetNearestDistanceToSideWays(const RnRef_t<RnLineString>& LineString, float& OutDistance) const { return false; }

    // 境界線の向きをそろえる
    virtual void AlignLaneBorder() {}

    // 境界線を調整するための線分を取得する
    virtual bool TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const { return false; }

    // 指定したレーンの境界線を取得する
    virtual RnRef_t<RnWay> GetBorderWay(const RnRef_t<RnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const { return nullptr; }

    // 指定したレーンの境界線を取得する
    virtual RnRef_t<RnWay> GetBorderWay(const RnRef_t<RnLane>& Lane, ERnLaneBorderType BorderType) const { return nullptr; }

    // RnRoadへキャストする
    virtual RnRef_t<RnRoad> CastToRoad() = 0;

    // RnIntersectionへキャストする
    virtual RnRef_t<RnIntersection> CastToIntersection() = 0;
};
