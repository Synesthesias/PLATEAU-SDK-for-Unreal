
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ICrosswalkPlacementRule.h"
#include "RoadMarkingGenerator.generated.h"

class IRrTarget;
class RoadReproduceSource;
class RoadMarkingInstance;

UCLASS()
class PLATEAURUNTIME_API URoadMarkingGenerator : public UObject
{
    GENERATED_BODY()

public:
    URoadMarkingGenerator();
    
    void Initialize(IRrTarget* Target, ECrosswalkFrequency CrosswalkFreq);
    void Generate();

private:
    TArray<TSharedPtr<RoadMarkingInstance>> GenerateRoadLines(IRrTarget* InnerTarget);
    
    //void GenerateGameObj(UStaticMesh* Mesh, 
    //                    const TArray<UMaterialInterface*>& Materials, 
    //                    USceneComponent* DstParent,
    //                    const RoadReproduceSource& SrcRoad,
    //                    EReproducedRoadType ReproducedType,
    //                    EReproducedRoadDirection Direction);

    IRrTarget* TargetBeforeCopy;
    ECrosswalkFrequency CrosswalkFrequency;
};
