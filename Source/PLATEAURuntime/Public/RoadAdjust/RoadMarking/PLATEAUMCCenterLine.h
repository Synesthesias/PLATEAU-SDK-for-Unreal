// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUMarkedWayListComposerMain.h"
#include "RoadAdjust/RoadMarking/PLATEAUMarkedWay.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnWay.h"
#include "PLATEAUMCCenterLine.generated.h"

/**
 * 交差点までの距離を計算するヘルパークラスです。
 */
class FPLATEAUIntersectionDistCalc
{
public:
    explicit FPLATEAUIntersectionDistCalc(URnRoad* Road);

    float NearestDistFromIntersection(const URnWay* Way, int32 WayIndexOrig) const;
    float GetLengthBetweenCenterLine() const { return LengthBetweenIntersections; }

private:
    /**
     * 道路の長さを返します。道路でない場合は0.0fを返します。
     */
    float RoadLength(URnRoadBase* RoadBase) const;

    float PrevLength;
    float NextLength;
    float LengthBetweenIntersections;
};

/**
 * 道路ネットワークから、センターラインを収集するクラスです。
 * センターラインとは、車の進行方向が違う車線を区切る線です。
 * MCはMarkedWayComposerの略です。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAUMCCenterLine : public UObject, public IPLATEAUMarkedWayListComposer
{
    GENERATED_BODY()

public:
    /**
     * 道路ネットワークからセンターラインを収集します。
     * @param Target 道路ネットワークのターゲット
     * @return 収集されたセンターラインのリスト
     */
    virtual FPLATEAUMarkedWayList ComposeFrom(const IPLATEAURrTarget* Target) override;

private:
    /** センターラインが黄色となる条件に合致するかどうかを返します。 */
    bool IsCenterLineYellow(float DistFromIntersection, float LengthBetweenIntersections) const;

    /** 片側の道路幅からセンターラインのタイプを判定します。 */
    EPLATEAUMarkedWayType GetCenterLineTypeOfWidth(const URnRoad* Road) const;

    /**
     * 線の中心に点を挿入したRnWayを新たに作って返します。
     * これにより、隣り合う点で近い交差点が違うケースを考慮せずにすみます。
     */
    URnWay* WayWithMiddlePoint(const URnWay* Way) const;

    static constexpr float WidthThreshold = 600.0f;                 // センターラインのタイプが変わるしきい値、道路の片側の幅
    static constexpr float YellowIntersectionThreshold = 3000.0f;   // 交差点との距離が近いかどうかのしきい値
    static constexpr float YellowRoadLengthThreshold = 10000.0f;    // この長さ以下の道路(交差点に挟まれた範囲)は、センターラインは白色
};
