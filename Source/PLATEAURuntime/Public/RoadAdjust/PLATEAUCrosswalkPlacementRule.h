// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "PLATEAUCrosswalkPlacementRule.generated.h"

/**
* @brief 横断歩道を置く頻度設定
*/
UENUM(BlueprintType)
enum class EPLATEAUCrosswalkFrequency : uint8 {
    /** 太くて長い道路に置く */
    BigRoad UMETA(DisplayName = "BigRoad"),
    /** 全ての交差点に置く */
    All UMETA(DisplayName = "All"),
    /** 置かない。すでに置かれているものは削除しない */
    None UMETA(DisplayName = "None"),
    /** 置かない。すでに置かれているものは削除する。 */
    Delete UMETA(DisplayName = "Delete"),
};

/**
* @brief 横断歩道を設置する条件を表す基底クラスです。
* 設置すべきときにメソッドShouldPlaceがtrueとなるようにサブクラスを実装します。
*/
class PLATEAURUNTIME_API IPLATEAUCrosswalkPlacementRule {
public:
    virtual ~IPLATEAUCrosswalkPlacementRule() = default;
    virtual bool ShouldPlace(const TRnRef_T<URnRoad>& Road) = 0;
};

/**
* @brief 横断歩道を設置する条件であり、太く長い道路であるときにtrueを返します。
*/
class PLATEAURUNTIME_API FPLATEAUCrosswalkPlacementRuleBigRoad : public IPLATEAUCrosswalkPlacementRule {
public:
    virtual bool ShouldPlace(const TRnRef_T<URnRoad>& Road) override;

private:
    /** レーン数がこの値未満の道路には設置しません */
    static constexpr int32 LaneCountThreshold = 2;
    /** 道路の長さがこの値未満の道路には設置しません */
    static constexpr float LengthThreshold = 3000.0f; // 30m * 100 (Unreal units)
};

/**
* @brief 横断歩道を設置する条件であり、全交差点に設置します。
*/
class PLATEAURUNTIME_API FPLATEAUCrosswalkPlacementRuleAll : public IPLATEAUCrosswalkPlacementRule {
public:
    virtual bool ShouldPlace(const TRnRef_T<URnRoad>& Road) override;
};

/**
* @brief 横断歩道を設置しません
*/
class PLATEAURUNTIME_API FPLATEAUCrosswalkPlacementRuleNone : public IPLATEAUCrosswalkPlacementRule {
public:
    virtual bool ShouldPlace(const TRnRef_T<URnRoad>& Road) override;
};

/**
* @brief 横断歩道を設置しません。すでに設置されている場合は削除します。
*/
class PLATEAURUNTIME_API FPLATEAUCrosswalkPlacementRuleDelete : public IPLATEAUCrosswalkPlacementRule {
public:
    virtual bool ShouldPlace(const TRnRef_T<URnRoad>& Road) override;
};

/**
* @brief CrosswalkFrequencyの拡張メソッド
*/
class PLATEAURUNTIME_API FPLATEAUCrosswalkFrequencyExtensions {
public:
    static TSharedPtr<IPLATEAUCrosswalkPlacementRule> ToPlacementRule(EPLATEAUCrosswalkFrequency Frequency);
};
