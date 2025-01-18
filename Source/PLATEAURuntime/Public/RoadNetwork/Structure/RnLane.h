#pragma once

#include "CoreMinimal.h"
#include "RnWay.h"
#include "RnRoad.h"
#include "../RnDef.h"
#include "RnLane.generated.h"

class URnRoad;
UCLASS()
class URnLane : public UObject
{
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
    ERnLaneBorderDir GetBorderDir(ERnLaneBorderType Type) const;

    // 境界線を取得する
    TRnRef_T<URnWay> GetBorder(ERnLaneBorderType Type) const;

    // 境界線を設定する
    void SetBorder(ERnLaneBorderType Type, const TRnRef_T<URnWay>& Border);

    // 指定した側のWayを取得する
    TRnRef_T<URnWay> GetSideWay(ERnDir Dir) const;

    // 指定した側のWayを設定する
    void SetSideWay(ERnDir Dir, const TRnRef_T<URnWay>& Way);

    // 車線の幅を計算する
    float CalcWidth() const;

    // 反転する
    void Reverse();

    // 中心線を生成する
    void BuildCenterWay();

    // 中心線を取得する
    TRnRef_T<URnWay> GetCenterWay();

    // 指定した点に最も近い中心線上の点を取得する
    void GetNearestCenterPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;

    // 中心線の長さを取得する
    float GetCenterLength() const;

    // 中心線の2D平面における長さを取得する
    float GetCenterLength2D(EAxisPlane Plane = FRnDef::Plane) const;

    // 中心線の2D平面における角度の合計を取得する
    float GetCenterTotalAngle2D() const;

    // 中心線の2D平面における曲率を取得する
    float GetCenterCurvature2D() const;

    // 中心線の2D平面における曲率半径を取得する
    float GetCenterRadius2D() const;

    // 中心線の2D平面における曲率半径の逆数を取得する
    float GetCenterInverseRadius2D() const;

    // 指定した点からの距離を取得する
    float GetDistanceFrom(const FVector& Point) const;

    // 指定した点が車線の内側にあるかどうかを取得する
    bool IsInside(const FVector& Point) const;

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

public:
    // 親リンク
    UPROPERTY()
    TObjectPtr<URnRoad> Parent;

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
    bool IsReverse;

    // 内部的に持つだけ. 中心線
    UPROPERTY()
    TObjectPtr<URnWay> CenterWay;

};
