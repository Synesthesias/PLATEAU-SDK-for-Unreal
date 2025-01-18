#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/GeoGraph/LineSegment3D.h"
#include "RnRoadBase.generated.h"

class URnModel;
class URnSideWalk;
class RnBorder;
class URnWay;
class URnLineString;
class URnLane;
class URnRoad;
class URnIntersection;

UCLASS()
class URnRoadBase : public UObject
{
    GENERATED_BODY()
public:
    URnRoadBase();
    virtual ~URnRoadBase() = default;

    // 歩道sideWalkを追加する
    // sideWalkの親情報も書き換える
    void AddSideWalk(const TRnRef_T<URnSideWalk>& SideWalk);

    // 歩道sideWalkを削除する
    // sideWalkの親情報は変更しない
    void RemoveSideWalk(const TRnRef_T<URnSideWalk>& SideWalk);

    // 境界線情報を取得
    virtual TArray<TRnRef_T<URnWay>> GetBorders() const { return TArray<TRnRef_T<URnWay>>(); }

    // 隣接するRoadを取得
    virtual TArray<TRnRef_T<URnRoadBase>> GetNeighborRoads() const { return TArray<TRnRef_T<URnRoadBase>>(); }

    // 対象のTargetTranを追加
    void AddTargetTran(UPLATEAUCityObjectGroup* TargetTran);
    void AddTargetTrans(const TArray<UPLATEAUCityObjectGroup*>& InTargetTrans);

    // 所属するすべてのWayを取得(重複の可能性あり)
    virtual TArray<TRnRef_T<URnWay>> GetAllWays() const;

    // otherをつながりから削除する. other側の接続は消えない
    virtual void UnLink(const TRnRef_T<URnRoadBase>& Other) {}

    // 自身の接続を切断する
    // removeFromModel=trueの場合、RnModelからも削除する
    virtual void DisConnect(bool RemoveFromModel);

    // デバッグ用) その道路の中心を表す代表頂点を返す
    virtual FVector GetCentralVertex() const { return FVector::ZeroVector; }

    virtual void ReplaceNeighbor(const TRnRef_T<URnRoadBase>& From, const TRnRef_T<URnRoadBase>& To) {}

    // 相互に接続を解除する
    void UnLinkEachOther(const TRnRef_T<URnRoadBase>& Other);

    // デバッグ表示用. TargetTransの名前を取得
    FString GetTargetTransName() const;

    // selfのすべてのLineStringを取得
    TSet<TRnRef_T<URnLineString>> GetAllLineStringsDistinct() const;

    // 歩道を取得
    const TArray<TRnRef_T<URnSideWalk>>& GetSideWalks() const { return SideWalks; }
    TArray<TRnRef_T<URnSideWalk>>& GetSideWalks() { return SideWalks; }

    const TArray<UPLATEAUCityObjectGroup*>& GetTargetTrans() const { return TargetTrans; }
    TArray<UPLATEAUCityObjectGroup*>& GetTargetTrans() { return TargetTrans; }

    TRnRef_T<URnModel> GetParentModel() const { return ParentModel; }
    void SetParentModel(const TRnRef_T<URnModel>& InParentModel) { ParentModel = InParentModel; }

    // 指定した方向の境界線を取得する
    virtual TRnRef_T<URnWay> GetMergedBorder(ERnLaneBorderType BorderType, std::optional<ERnDir> Dir) const { return nullptr; }

    // 指定した方向のWayを取得する
    virtual TRnRef_T<URnWay> GetMergedSideWay(ERnDir Dir) const { return nullptr; }

    // 指定した方向のWayを取得する
    virtual bool TryGetMergedSideWay(ERnDir Dir, TRnRef_T<URnWay>& OutLeftWay, TRnRef_T<URnWay>& OutRightWay) const { return false; }

    // 指定したLineStringまでの最短距離を取得する
    virtual bool TryGetNearestDistanceToSideWays(const TRnRef_T<URnLineString>& LineString, float& OutDistance) const { return false; }

    // 境界線の向きをそろえる
    virtual void AlignLaneBorder() {}

    // 境界線を調整するための線分を取得する
    virtual bool TryGetAdjustBorderSegment(ERnLaneBorderType BorderType, FLineSegment3D& OutSegment) const { return false; }

    // 指定したレーンの境界線を取得する
    virtual TRnRef_T<URnWay> GetBorderWay(const TRnRef_T<URnLane>& Lane, ERnLaneBorderType BorderType, ERnLaneBorderDir Dir) const { return nullptr; }

    // 指定したレーンの境界線を取得する
    virtual TRnRef_T<URnWay> GetBorderWay(const TRnRef_T<URnLane>& Lane, ERnLaneBorderType BorderType) const { return nullptr; }

    // RnRoadへキャストする
    virtual TRnRef_T<URnRoad> CastToRoad()
    {
        return nullptr;
    }

    // RnIntersectionへキャストする
    virtual TRnRef_T<URnIntersection> CastToIntersection()
    {
        return nullptr;
    }

private:

    // 自分が所属するRoadNetworkModel
    UPROPERTY()
    TObjectPtr<URnModel> ParentModel;

    // これに紐づくtranオブジェクトリスト(統合なので複数存在する場合がある)
    UPROPERTY()
    TArray<UPLATEAUCityObjectGroup*> TargetTrans;

    // 歩道情報
    UPROPERTY()
    TArray<TObjectPtr<URnSideWalk>> SideWalks;

};
