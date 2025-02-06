
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IRrTarget.h"
#include "../RoadNetwork/Structure/RnModel.h"
//#include "RnRoad.h"
//#include "RnIntersection.h"
#include "RrTargetModel.generated.h"

/**
 * IRrTargetで、1つの道路モデル全体を対象に取ります。
 */
UCLASS()
class PLATEAURUNTIME_API URrTargetModel : public UObject, public IIRrTarget
{
    GENERATED_BODY()

public:
    URrTargetModel();
    
    void Initialize(TObjectPtr<class URnModel> InModel);
    
    // IRrTarget interface
    virtual TArray<URnRoadBase*> RoadBases() override;
    virtual TArray<URnRoad*> Roads() override;
    virtual TArray<URnIntersection*> Intersections() override;
    virtual TScriptInterface<IIRrTarget> Copy() override;
    virtual TObjectPtr<class URnModel> Network() override;
    
private:
    UPROPERTY()
    TObjectPtr<URnModel> Model;
};
