// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/MarkedWayList.h"

UMarkedWayList::UMarkedWayList()
{
    Ways.Empty();
}

void UMarkedWayList::Add(UMarkedWay* Way)
{
    if (!Way || !Way->GetLine())
    {
        UE_LOG(LogTemp, Warning, TEXT("way is null."));
        return;
    }
    Ways.Add(Way);
}

void UMarkedWayList::AddRange(UMarkedWayList* WayList)
{
    if (!WayList)
    {
        UE_LOG(LogTemp, Warning, TEXT("wayList is null."));
        return;
    }

    for (UMarkedWay* Way : WayList->Ways)
    {
        Add(Way);
    }
}

void UMarkedWayList::Translate(FVector Diff)
{
    for (UMarkedWay* Way : Ways)
    {
        if (Way)
        {
            Way->Translate(Diff);
        }
    }
}
