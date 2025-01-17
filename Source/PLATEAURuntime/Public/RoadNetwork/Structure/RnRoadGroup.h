#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <optional>
#include "RnRoadGroup.generated.h"
class URnIntersection;
class URnRoad;
class URnSideWalk;
class URnWay;
class URnLane;
class URnLineString;

UCLASS()
class URnRoadGroup : public UObject
{
    GENERATED_BODY()
public:
    // 開始ノード
    TRnRef_T<URnIntersection> PrevIntersection;

    // 終了ノード
    TRnRef_T<URnIntersection> NextIntersection;

    // 道路リスト
    TSharedPtr<TArray<TRnRef_T<URnRoad>>> Roads;
    URnRoadGroup() = default;
    URnRoadGroup(TRnRef_T<URnIntersection> InPrevIntersection,
        TRnRef_T<URnIntersection> InNextIntersection,
        const TArray<TRnRef_T<URnRoad>>& InRoads);

    void Init(TRnRef_T<URnIntersection> InPrevIntersection,
        TRnRef_T<URnIntersection> InNextIntersection,
        const TArray<TRnRef_T<URnRoad>>& InRoads);

    // 有効なRoadGroupかどうか
    bool IsValid() const;

    // 左側のレーン数を取得
    int32 GetLeftLaneCount() const;

    // 右側のレーン数を取得
    int32 GetRightLaneCount() const;

    // 右側のレーンを取得
    TArray<TRnRef_T<URnLane>> GetRightLanes() const;

    // 左側のレーンを取得
    TArray<TRnRef_T<URnLane>> GetLeftLanes() const;
public:
    // RnDirで指定した側のレーンを取得
    TArray<TRnRef_T<URnLane>> GetLanes(ERnDir Dir) const;

    // 中央分離帯を取得
    void GetMedians(TArray<TRnRef_T<URnWay>>& OutLeft, TArray<TRnRef_T<URnWay>>& OutRight) const;

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
    static TRnRef_T<URnRoadGroup> CreateRoadGroupOrDefault(TRnRef_T<URnIntersection> PrevIntersection, TRnRef_T<URnIntersection> NextIntersection);
    static bool IsSameRoadGroup(TRnRef_T<URnRoadGroup> A, TRnRef_T<URnRoadGroup> B);

private:


    TSharedPtr<TMap<TRnRef_T<URnRoad>, TArray<TRnRef_T<URnLane>>>> SplitLane(
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
