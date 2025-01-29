// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMarkedWayListComposer.generated.h"

class IRrTarget;
class UMarkedWayList;

UINTERFACE(MinimalAPI)
class UIMarkedWayListComposer : public UInterface
{
    GENERATED_BODY()
};

class PLATEAURUNTIME_API IIMarkedWayListComposer
{
    GENERATED_BODY()

public:
    virtual class TSharedPtr<UMarkedWayList> ComposeFrom(const TSharedPtr<IRrTarget>& Target) = 0;

};
