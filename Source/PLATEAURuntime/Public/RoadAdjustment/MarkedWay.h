// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MarkedWay.generated.h"

class UMWLine;

/// <summary> 道路の線を描くにあたって見た目が異なるタイプのenumです。 </summary>
UENUM()
enum class EMarkedWayType : uint8
{
    /// <summary> センターライン、すなわち、車の進行方向が違う車線を区切る線。6メートル以上か以下かで線のタイプが異なる。 </summary>
    CenterLineOver6MWidth,
    CenterLineUnder6MWidth,
    CenterLineNearIntersection,
    /// <summary> 車の進行方向が同じ車線を区切る線。すなわち、車線同士を区切る線のうち、センターラインでない線。 車線境界線。</summary>
    LaneLine,
    /// <summary> 車道と歩道の間の線。路側帯線。 </summary>
    ShoulderLine,
    /// <summary> 停止線 </summary>
    StopLine,
    None
};

/**
 * Class representing a single road marking line
 */
UCLASS()
class PLATEAURUNTIME_API UMarkedWay : public UObject
{
    GENERATED_BODY()

public:
    UMarkedWay();

    void Initialize(TObjectPtr<UMWLine> InLine, EMarkedWayType InType, bool bInIsReversed);
    void Translate(const FVector& Diff);

    // Getters
    TObjectPtr<UMWLine> GetLine() const { return Line; }
    EMarkedWayType GetType() const { return Type; }
    bool IsReversed() const { return bIsReversed; }

private:
    UPROPERTY()
    TObjectPtr<UMWLine> Line;

    UPROPERTY()
    EMarkedWayType Type;

    UPROPERTY()
    bool bIsReversed; // Equivalent to RnLane.IsReverse
};
