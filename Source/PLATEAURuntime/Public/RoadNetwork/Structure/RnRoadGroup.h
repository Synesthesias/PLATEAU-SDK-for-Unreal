#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <optional>
class RnIntersection;
class RnRoad;
class RnSideWalk;
class RnWay;
class RnLane;
class RnLineString;

class RnRoadGroup
{
public:
    // 開始ノード
    RnRef_t<RnIntersection> PrevIntersection;

    // 終了ノード
    RnRef_t<RnIntersection> NextIntersection;

    // 道路リスト
    TSharedPtr<TArray<RnRef_t<RnRoad>>> Roads;

    RnRoadGroup(RnRef_t<RnIntersection> InPrevIntersection,
        RnRef_t<RnIntersection> InNextIntersection,
        const TArray<RnRef_t<RnRoad>>& InRoads);

    // 有効なRoadGroupかどうか
    bool IsValid() const;

    // 左側のレーン数を取得
    int32 GetLeftLaneCount() const;

    // 右側のレーン数を取得
    int32 GetRightLaneCount() const;

    // 右側のレーンを取得
    TArray<RnRef_t<RnLane>> GetRightLanes() const;

    // 左側のレーンを取得
    TArray<RnRef_t<RnLane>> GetLeftLanes() const;
public:
    // RnDirで指定した側のレーンを取得
    TArray<RnRef_t<RnLane>> GetLanes(ERnDir Dir) const;

    // 中央分離帯を取得
    void GetMedians(TArray<RnRef_t<RnWay>>& OutLeft, TArray<RnRef_t<RnWay>>& OutRight) const;

    // 中央分離帯があるかどうか
    bool HasMedians() const;

    // 中央分離帯を作成する
    bool CreateMedianOrSkip(float MedianWidth = 1.0f, float MaxMedianLaneRate = 0.5f);

    // 中央分離帯があるかどうか
    bool HasMedian() const;

public:
    // 向きがそろっているかどうか
    bool IsAligned() const;

    // 複数道路がきれいに整列されているかどうか
    bool IsDeepAligned() const;

    // LaneのPrev/Nextの向きをそろえる
    bool Align();

    // LaneのPrev/Nextの向きをそろえる
    bool Align() const;
    // 複数のRoadsを1つのRoadにまとめる
    bool MergeRoads();

    // 交差点との境界線の角度を調整する
    void AdjustBorder();

    // Static Methods
    static RnRef_t<RnRoadGroup> CreateRoadGroupOrDefault(RnRef_t<RnIntersection> PrevIntersection, RnRef_t<RnIntersection> NextIntersection);
    static bool IsSameRoadGroup(RnRef_t<RnRoadGroup> A, RnRef_t<RnRoadGroup> B);

private:


    TSharedPtr<TMap<RnRef_t<RnRoad>, TArray<RnRef_t<RnLane>>>> SplitLane(
        int32 Num,
        std::optional<ERnDir> Dir,
        // #TODO : nullptr入れられるのか確認
        TFunction<float(int32)> GetSplitRate = nullptr);

    // レーン分割する
    void SetLaneCountImpl(int32 Count, ERnDir Dir, bool RebuildTrack);

    // レーン数を変更する
    void SetLaneCountWithoutMedian(int32 LeftCount, int32 RightCount, bool RebuildTrack);

public:
    // レーン数を変更する
    void SetLaneCount(int32 LeftCount, int32 RightCount, bool RebuildTrack = true);

    // 左側レーン数を変更する
    void SetLeftLaneCount(int32 Count, bool RebuildTrack = true);

    // 右側レーン数を変更する
    void SetRightLaneCount(int32 Count, bool RebuildTrack = true);

    // 指定した側のレーン数を設定する
    void SetLaneCount(ERnDir Dir, int32 Count, bool RebuildTrack = true);

    // 中央分離帯を考慮したレーン分割
    void SetLaneCountWithMedian(int32 LeftCount, int32 RightCount, float MedianWidthRate);

};
