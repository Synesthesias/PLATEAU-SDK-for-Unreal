#pragma once

#include "CoreMinimal.h"
#include "RnWay.h"
#include "RnRoad.h"
#include "../RnDef.h"

class RnRoad;
class RnLane {
public:
    RnLane();
    RnLane(const RnRef_t<RnWay>& LeftWay, const RnRef_t<RnWay>& RightWay,
        const RnRef_t<RnWay>& PrevBorder, const RnRef_t<RnWay>& NextBorder);

    // 親リンク
    RnRef_t<RnRoad> Parent;

    // 境界線(下流)
    RnRef_t<RnWay> PrevBorder;

    // 境界線(上流)
    RnRef_t<RnWay> NextBorder;

    // 車線(左)
    RnRef_t<RnWay> LeftWay;

    // 車線(右)
    RnRef_t<RnWay> RightWay;

    // 親Roadと逆方向(右車線等)
    bool IsReverse;

    // 内部的に持つだけ. 中心線
    RnRef_t<RnWay> CenterWay;

    // Left/Right両方のWayを返す(nullの物は含まない)
    TArray<RnRef_t<RnWay>> GetBothWays() const;

    // Prev/Nextの境界線を返す(nullの物は含まない)
    TArray<RnRef_t<RnWay>> GetAllBorders() const;

    // Border/Side両方合わせた全てのWayを返す
    TArray<RnRef_t<RnWay>> GetAllWays() const;

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
    RnRef_t<RnWay> GetBorder(ERnLaneBorderType Type) const;

    // 境界線を設定する
    void SetBorder(ERnLaneBorderType Type, const RnRef_t<RnWay>& Border);

    // 指定した側のWayを取得する
    RnRef_t<RnWay> GetSideWay(ERnDir Dir) const;

    // 指定した側のWayを設定する
    void SetSideWay(ERnDir Dir, const RnRef_t<RnWay>& Way);

    // 車線の幅を計算する
    float CalcWidth() const;

    // 反転する
    void Reverse();

    // 中心線を生成する
    void BuildCenterWay();

    // 中心線を取得する
    RnRef_t<RnWay> GetCenterWay();

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
    RnRef_t<RnLane> Clone() const;

};
