
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ICrosswalkPlacementRule.generated.h"

class RnRoad;

UENUM()
enum class ECrosswalkFrequency : uint8
{
    BigRoad     UMETA(DisplayName = "BigRoad"),
    All         UMETA(DisplayName = "All"),
    None        UMETA(DisplayName = "None"),
    Delete      UMETA(DisplayName = "Delete")
};

UINTERFACE(MinimalAPI)
class UICrosswalkPlacementRule : public UInterface
{
    GENERATED_BODY()
};

class PLATEAURUNTIME_API IICrosswalkPlacementRule
{
    GENERATED_BODY()

public:
    virtual bool ShouldPlace(const RnRoad* Road) = 0;
};

UCLASS()
class PLATEAURUNTIME_API UCrosswalkPlacementRuleBigRoad : public UObject, public IICrosswalkPlacementRule
{
    GENERATED_BODY()

public:
    virtual bool ShouldPlace(const RnRoad* Road) override;

private:
    static constexpr int32 LaneCountThreshold = 2;
    static constexpr float LengthThreshold = 30.0f;
};

UCLASS()
class PLATEAURUNTIME_API UCrosswalkPlacementRuleAll : public UObject, public IICrosswalkPlacementRule
{
    GENERATED_BODY()

public:
    virtual bool ShouldPlace(const RnRoad* Road) override;
};

UCLASS()
class PLATEAURUNTIME_API UCrosswalkPlacementRuleNone : public UObject, public IICrosswalkPlacementRule
{
    GENERATED_BODY()

public:
    virtual bool ShouldPlace(const RnRoad* Road) override;
};

UCLASS()
class PLATEAURUNTIME_API UCrosswalkPlacementRuleDelete : public UObject, public IICrosswalkPlacementRule
{
    GENERATED_BODY()

public:
    virtual bool ShouldPlace(const RnRoad* Road) override;
};

class PLATEAURUNTIME_API FCrosswalkFrequencyExtensions
{
public:
    static TScriptInterface<IICrosswalkPlacementRule> ToPlacementRule(ECrosswalkFrequency Frequency);
};
