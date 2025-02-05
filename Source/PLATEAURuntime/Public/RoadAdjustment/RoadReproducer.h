
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IRrTarget.h"
//#include "RoadNetworkToMesh.h"
//#include "RoadMarking.h"


#include "RoadAdjustment/ICrosswalkPlacementRule.h"

#include "RoadReproducer.generated.h"

UCLASS()
class PLATEAURUNTIME_API URoadReproducer : public UObject {
    GENERATED_BODY()

public:
    /** 道路ネットワークからの道路メッシュ生成(RoadNetworkToMesh)と
     * 道路標示生成(RoadMarking)を合わせて
     * よりよい見た目の道路を生成します。
     */
    URoadReproducer();

    void Generate(class IRrTarget* Target, ECrosswalkFrequency CrosswalkFrequency);

    static class USceneComponent* GenerateDstParent();
};
