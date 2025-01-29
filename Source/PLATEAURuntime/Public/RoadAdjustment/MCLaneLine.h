// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IMarkedWayListComposer.h"
#include "MCLaneLine.generated.h"

UCLASS()
class PLATEAURUNTIME_API UMCLaneLine : public UObject, public IIMarkedWayListComposer
{
    GENERATED_BODY()

public:
    virtual class TSharedPtr<UMarkedWayList> ComposeFrom(const TSharedPtr<IRrTarget>& Target) override;
};
