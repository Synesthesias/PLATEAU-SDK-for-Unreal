// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMarkedWayListComposer.h"
#include "MCShoulderLine.generated.h"

/**
 * Collects shoulder lines (ShoulderLine) from the road network.
 * Shoulder lines refer to the lines between the roadway and sidewalk.
 * MC stands for MarkedWayComposer.
 */
UCLASS()
class PLATEAURUNTIME_API UMCShoulderLine : public UObject, public IIMarkedWayListComposer
{
    GENERATED_BODY()
    
public:
    /** Composes MarkedWayList from the given road target */
    virtual class TSharedPtr<UMarkedWayList> ComposeFrom(const TSharedPtr<IRrTarget>& Target) override;

};
