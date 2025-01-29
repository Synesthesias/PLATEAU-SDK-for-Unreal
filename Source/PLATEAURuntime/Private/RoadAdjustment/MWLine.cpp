// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/MWLine.h"

UMWLine::UMWLine()
{
}

UMWLine::UMWLine(const TArray<FVector>& InPoints)
    : Points(InPoints)
{
}

UMWLine::~UMWLine()
{
}

float UMWLine::SumDistance() const
{
    if (Points.Num() == 0) return 0.0f;
    
    float Length = 0.0f;
    for (int32 i = 1; i < Points.Num(); i++)
    {
        Length += FVector::Distance(Points[i], Points[i-1]);
    }
    
    return Length;
}
