
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IRrTarget.h"
#include "RrTargetRoadBases.generated.h"

UCLASS()
class PLATEAURUNTIME_API URrTargetRoadBases : public UObject, public IIRrTarget
{
    GENERATED_BODY()

public:
    URrTargetRoadBases();

    void Initialize(URnModel* InNetwork, const TArray<URnRoadBase*>& InRoadBases);
    
    virtual TArray<URnRoadBase*> RoadBases() override;
    virtual TArray<URnRoad*> Roads() override;
    virtual TArray<URnIntersection*> Intersections() override; 
    virtual TScriptInterface<IIRrTarget> Copy() override;
    virtual TObjectPtr<class URnModel> Network() override { return NetworkModel; };

private:
    URnModel* NetworkModel;
    TArray<URnRoadBase*> RoadBasesArray;
};
