// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "RoadAdjust/PLATEAUCrosswalkPlacementRule.h"
#include "RoadAdjust/PLATEAUReproducedRoad.h"
#include "RoadNetwork/Structure/RnLane.h"

bool FPLATEAUCrosswalkPlacementRuleBigRoad::ShouldPlace(const TRnRef_T<URnRoad>& Road) {
    // レーン数があること
    if (Road->GetMainLanes().Num() < LaneCountThreshold) return false;

    // 十分長いこと
    const auto& Lane = Road->GetMainLanes()[0];
    const float LeftLength = Lane->GetLeftWay()->IsValid() ? Lane->GetLeftWay()->CalcLength() : 0.0f;
    const float RightLength = Lane->GetRightWay()->IsValid() ? Lane->GetRightWay()->CalcLength() : 0.0f;
    return FMath::Max(LeftLength, RightLength) >= LengthThreshold;
}

bool FPLATEAUCrosswalkPlacementRuleAll::ShouldPlace(const TRnRef_T<URnRoad>& Road) {
    return true;
}

bool FPLATEAUCrosswalkPlacementRuleNone::ShouldPlace(const TRnRef_T<URnRoad>& Road) {
    return false;
}

bool FPLATEAUCrosswalkPlacementRuleDelete::ShouldPlace(const TRnRef_T<URnRoad>& Road) {

    // Unityから移植途中のためいったんコメントアウト
    
    // const auto Target = MakeShared<FRoadReproduceSource>(Road);
    //
    // auto CrosswalkA = APLATEAUReproducedRoad::Find(
    //     EPLATEAUReproducedRoadType::Crosswalk,
    //     Target,
    //     EPLATEAUReproducedRoadDirection::Next);
    //     
    // auto CrosswalkB = APLATEAUReproducedRoad::Find(
    //     EPLATEAUReproducedRoadType::Crosswalk,
    //     Target,
    //     EPLATEAUReproducedRoadDirection::Prev);
    //
    // if (CrosswalkA != nullptr) CrosswalkA->Destroy();
    // if (CrosswalkB != nullptr) CrosswalkB->Destroy();

    return false;
}

TSharedPtr<IPLATEAUCrosswalkPlacementRule> FPLATEAUCrosswalkFrequencyExtensions::ToPlacementRule(EPLATEAUCrosswalkFrequency Frequency) {
    switch (Frequency) {
        case EPLATEAUCrosswalkFrequency::BigRoad:
            return MakeShared<FPLATEAUCrosswalkPlacementRuleBigRoad>();
        case EPLATEAUCrosswalkFrequency::All:
            return MakeShared<FPLATEAUCrosswalkPlacementRuleAll>();
        case EPLATEAUCrosswalkFrequency::None:
            return MakeShared<FPLATEAUCrosswalkPlacementRuleNone>();
        case EPLATEAUCrosswalkFrequency::Delete:
            return MakeShared<FPLATEAUCrosswalkPlacementRuleDelete>();
        default:
            checkf(false, TEXT("Invalid CrosswalkFrequency"));
            return nullptr;
    }
}
