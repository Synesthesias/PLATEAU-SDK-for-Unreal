// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MarkedWay.h"
#include "MarkedWayList.generated.h"

/**
 * List of MarkedWay objects
 */
UCLASS()
class PLATEAURUNTIME_API UMarkedWayList : public UObject
{
    GENERATED_BODY()

public:
    UMarkedWayList();
    
    void Add(UMarkedWay* Way);
    void AddRange(UMarkedWayList* WayList);
    void Translate(FVector Diff);
    
    UFUNCTION()
    int32 GetCount() const { return Ways.Num(); }
    
    const TArray<UMarkedWay*>& GetMarkedWays() const { return Ways; }

private:
    UPROPERTY()
    TArray<UMarkedWay*> Ways;
};
