
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/ICrosswalkPlacementRule.h"
//#include "RoadNetwork/RnRoad.h"
#include "Kismet/GameplayStatics.h"

bool UCrosswalkPlacementRuleBigRoad::ShouldPlace(const RnRoad* Road)
{
    //if (!Road || Road->GetMainLanes().Num() < LaneCountThreshold)
    //{
    //    return false;
    //}

    //const auto& Lane = Road->GetMainLanes()[0];
    //const float LeftLength = Lane->GetLeftWay() ? Lane->GetLeftWay()->CalcLength() : 0.0f;
    //const float RightLength = Lane->GetRightWay() ? Lane->GetRightWay()->CalcLength() : 0.0f;

    //return FMath::Max(LeftLength, RightLength) >= LengthThreshold;
    return false;
}

bool UCrosswalkPlacementRuleAll::ShouldPlace(const RnRoad* Road)
{
    return true;
}

bool UCrosswalkPlacementRuleNone::ShouldPlace(const RnRoad* Road)
{
    return false;
}

bool UCrosswalkPlacementRuleDelete::ShouldPlace(const RnRoad* Road)
{
    //if (!Road) return false;

    //// Note: Implementation of finding and destroying crosswalks would need to be adapted
    //// to match your specific Unreal Engine implementation
    //RoadReproduceSource Target(Road);
    //
    //auto* CrosswalkA = PLATEAUReproducedRoad::Find(EReproducedRoadType::Crosswalk, Target, EReproducedRoadDirection::Next);
    //auto* CrosswalkB = PLATEAUReproducedRoad::Find(EReproducedRoadType::Crosswalk, Target, EReproducedRoadDirection::Prev);

    //if (CrosswalkA)
    //{
    //    CrosswalkA->Destroy();
    //}
    //if (CrosswalkB)
    //{
    //    CrosswalkB->Destroy();
    //}

    //return false;
    return false;
}

TScriptInterface<IICrosswalkPlacementRule> FCrosswalkFrequencyExtensions::ToPlacementRule(ECrosswalkFrequency Frequency)
{
    UObject* NewRule = nullptr;
    switch (Frequency)
    {
    case ECrosswalkFrequency::BigRoad:
        NewRule = NewObject<UCrosswalkPlacementRuleBigRoad>();
        break;
    case ECrosswalkFrequency::All:
        NewRule = NewObject<UCrosswalkPlacementRuleAll>();
        break;
    case ECrosswalkFrequency::None:
        NewRule = NewObject<UCrosswalkPlacementRuleNone>();
        break;
    case ECrosswalkFrequency::Delete:
        NewRule = NewObject<UCrosswalkPlacementRuleDelete>();
        break;
    default:
        checkf(false, TEXT("Invalid CrosswalkFrequency value"));
        return TScriptInterface<IICrosswalkPlacementRule>();
    }

    return TScriptInterface<IICrosswalkPlacementRule>(NewRule);
}
