
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/PLATEAUReproducedRoad.h"
#include "Kismet/GameplayStatics.h"
#include "RoadNetwork/Structure/RnRoadBase.h"

FRoadReproduceSource::FRoadReproduceSource() : Transform(nullptr)
{
}

FRoadReproduceSource::FRoadReproduceSource(URnRoadBase* Road) : Transform(nullptr)
{
    if (Road != nullptr)
    {
        // Transform setting logic would go here
        // This needs to be adapted based on your RnRoadBase implementation
        
        // URnRoadBaseからId取得の処理がないため、以下を参考に-1を設定
        // | PLATEAUReproducedRoad.cs
        // | 元となるtransformが削除された場合、代わりにこちらを一致判定に使います。不明な場合は-1です。
        RoadNetworkID = -1; 
    }
}

FString FRoadReproduceSource::GetName() const
{
    if (!Transform)
        return TEXT("UnknownRoad");
    
    return Transform->GetName();
}

bool FRoadReproduceSource::IsSourceExists() const
{
    return Transform != nullptr;
}

bool FRoadReproduceSource::IsMatch(const FRoadReproduceSource& Other) const
{
    if (Transform == nullptr && Other.Transform == nullptr)
    {
        return RoadNetworkID == Other.RoadNetworkID && RoadNetworkID >= 0;
    }

    return Transform == Other.Transform;
}

APLATEAUReproducedRoad::APLATEAUReproducedRoad()
{
    PrimaryActorTick.bCanEverTick = false;
}

void APLATEAUReproducedRoad::Init(EReproducedRoadType RoadTypeArg, const FRoadReproduceSource& SourceRoadArg, EReproducedRoadDirection RoadDirectionArg)
{
    RoadType = RoadTypeArg;
    ReproduceSource = SourceRoadArg;
    RoadDirection = RoadDirectionArg;
}

AActor* APLATEAUReproducedRoad::Find(EReproducedRoadType RoadTypeArg, const FRoadReproduceSource& SourceRoadArg, EReproducedRoadDirection RoadDirectionArg)
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, APLATEAUReproducedRoad::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        APLATEAUReproducedRoad* Road = Cast<APLATEAUReproducedRoad>(Actor);
        if (Road)
        {
            bool Match = Road->RoadType == RoadTypeArg && 
                        Road->ReproduceSource.IsMatch(SourceRoadArg) && 
                        Road->RoadDirection == RoadDirectionArg;
            
            if (Match)
            {
                return Actor;
            }
        }
    }

    return nullptr;
}

FString APLATEAUReproducedRoad::GetGameObjName(EReproducedRoadType Type)
{
    switch (Type)
    {
    case EReproducedRoadType::RoadMesh:
        return TEXT("Road");
    case EReproducedRoadType::LaneLineAndArrow:
        return TEXT("LaneArrow");
    case EReproducedRoadType::Crosswalk:
        return TEXT("Crosswalk");
    default:
        checkf(false, TEXT("Invalid ReproducedRoadType"));
        return TEXT("");
    }
}
