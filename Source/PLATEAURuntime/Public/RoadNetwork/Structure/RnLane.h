#pragma once

#include "CoreMinimal.h"
#include "RnWay.h"
#include "RnRoad.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "RnLane.generated.h"

class URnRoad;
UCLASS()
class URnLane : public UObject
{
private:
    GENERATED_BODY()

public:
    URnLane();
    URnLane(const TRnRef_T<URnWay>& LeftWay, const TRnRef_T<URnWay>& RightWay,
        const TRnRef_T<URnWay>& PrevBorder, const TRnRef_T<URnWay>& NextBorder);
    void Init();
    void Init(const TRnRef_T<URnWay>& InLeftWay, const TRnRef_T<URnWay>& InRightWay,
              const TRnRef_T<URnWay>& InPrevBorder, const TRnRef_T<URnWay>& InNextBorder);

    // Left/Right両方のWayを返す(nullの物は含まない)
    TArray<TRnRef_T<URnWay>> GetBothWays() const;

    // Prev/Nextの境界線を返す(nullの物は含まない)
    TArray<TRnRef_T<URnWay>> GetAllBorders() const;

    // Border/Side両方合わせた全てのWayを返す
    TArray<TRnRef_T<URnWay>> GetAllWays() const;

    TRnRef_T<URnRoad> GetParent() const;

    void SetParent(TRnRef_T<URnRoad> InParent);

    bool GetIsReverse() const {
        return bIsReverse;
    }

    void SetIsReverse(bool bReverse) {
        bIsReverse = bReverse;
    }

    TRnRef_T<URnWay> GetLeftWay() const;

    TRnRef_T<URnWay> GetRightWay() const;

    TRnRef_T<URnWay> GetPrevBorder() const;

    TRnRef_T<URnWay> GetNextBorder() const;

    // 有効なレーンかどうか
    // Left/Rightどっちも有効ならtrue
    bool IsValidWay() const;

    // 道の両方に接続先があるかどうか
    bool IsBothConnectedLane() const;

    // 両方に境界線を持っている
    bool HasBothBorder() const;

    // 隣接した交差点に挿入された空レーンかどうか
    bool IsEmptyLane() const;

    // 自分が中央分離帯かどうか. 親がないときはfalseになる
    bool IsMedianLane() const;

    // 境界線の方向を取得する
    TOptional<EPLATEAURnLaneBorderDir> GetBorderDir(EPLATEAURnLaneBorderType Type) const;

    // 境界線を取得する
    TRnRef_T<URnWay> GetBorder(EPLATEAURnLaneBorderType Type) const;

    // 境界線を設定する
    void SetBorder(EPLATEAURnLaneBorderType Type, const TRnRef_T<URnWay>& Border);

    // 指定した側のWayを取得する
    TRnRef_T<URnWay> GetSideWay(EPLATEAURnDir Dir) const;

    // 指定した側のWayを設定する
    void SetSideWay(EPLATEAURnDir Dir, const TRnRef_T<URnWay>& Way);

    // 単純な車線の幅を計算する
    // Next/PrevBorderの短い方
    float CalcWidth() const;

    // PrevBorderの長さを返す
    float CalcPrevBorderWidth() const;

    // NextBorderの長さを返す
    float CalcNextBorderWidth() const;

    // このレーンのうち最も狭くなる場所の幅を返す.
    // 頂点ごとに計算するため割と重い
    // 左右のレーンが不正の場合は0を返す
    float CalcMinWidth() const;

    // 反転する
    void Reverse();

    // Borderの向きをborderDirになるようにそろえる
    void AlignBorder(EPLATEAURnLaneBorderDir borderDir = EPLATEAURnLaneBorderDir::Left2Right);

    // 中心線を生成する
    void BuildCenterWay();

    // 中心線を取得する
    TRnRef_T<URnWay> GetCenterWay();

    // 指定した点に最も近い中心線上の点を取得する
    void GetNearestCenterPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;

    // 中心線の長さを取得する
    float GetCenterLength() const;

    // 指定した点からの距離を取得する
    float GetDistanceFrom(const FVector& Point) const;

    // 指定した点が車線の内側にあるかどうかを取得する
    bool IsInside(const FVector& Point) const;

    FVector GetCentralVertex() const;

    // クローンを作成する
    TRnRef_T<URnLane> Clone() const;

    static TRnRef_T<URnLane> CreateOneWayLane(TRnRef_T<URnWay> way);

    /// <summary>
    /// 交差点同士の間に入れる空のレーンを作成
    /// </summary>
    /// <param name="border"></param>
    /// <param name="centerWay"></param>
    /// <returns></returns>
    static TRnRef_T<URnLane> CreateEmptyLane(TRnRef_T<URnWay> border, TRnRef_T<URnWay> centerWay);

private:

    // typeの境界線をborderDirにそろえる
    void AlignBorder(EPLATEAURnLaneBorderType type, EPLATEAURnLaneBorderDir borderDir);

private:
    // 親リンク
    UPROPERTY()
    TWeakObjectPtr<URnRoad> Parent;

    // 境界線(下流)
    UPROPERTY()
    TObjectPtr<URnWay> PrevBorder;

    // 境界線(上流)
    UPROPERTY()
    TObjectPtr<URnWay> NextBorder;

    // 車線(左)
    UPROPERTY()
    TObjectPtr<URnWay> LeftWay;

    // 車線(右)
    UPROPERTY()
    TObjectPtr<URnWay> RightWay;

    // 親Roadと逆方向(右車線等)
    UPROPERTY()
    bool bIsReverse;

    // 内部的に持つだけ. 中心線
    UPROPERTY()
    TObjectPtr<URnWay> CenterWay;

};

struct FRnLaneEx
{
};