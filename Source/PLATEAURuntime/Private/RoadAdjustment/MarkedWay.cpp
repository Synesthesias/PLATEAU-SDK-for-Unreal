// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/MarkedWay.h"
#include "RoadAdjustment/MWLine.h"

UMarkedWay::UMarkedWay()
    : Line(nullptr)
    , Type(EMarkedWayType::None) // Assuming you have a None type
    , bIsReversed(false)
{
}

void UMarkedWay::Initialize(TObjectPtr<UMWLine> InLine, EMarkedWayType InType, bool bInIsReversed)
{
    Line = InLine;
    Type = InType;
    bIsReversed = bInIsReversed;
}

void UMarkedWay::Translate(const FVector& Diff)
{
    if (Line)
    {
        // Assuming MWLine has a method to translate its points
        //Line->TranslatePoints(Diff);
    }
}
