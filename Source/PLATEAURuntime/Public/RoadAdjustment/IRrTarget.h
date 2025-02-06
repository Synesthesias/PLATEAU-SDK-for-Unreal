
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IRrTarget.generated.h"

UINTERFACE(MinimalAPI)
class UIRrTarget : public UInterface
{
    GENERATED_BODY()
};

/**
 * 道路ネットワークから道路モデルを生成する処理において、どの部分を対象にするのかを記述します。
 * 対象が道路ネットワーク全体か、指定の道路かを切り替えます。
 * Rrは RoadReproducer の略です。
 */
class PLATEAURUNTIME_API IIRrTarget
{
    GENERATED_BODY()

public:
    virtual TArray<class URnRoadBase*> RoadBases() = 0;
    
    virtual TArray<class URnRoad*> Roads() = 0;
    
    virtual TArray<class URnIntersection*> Intersections() = 0;
    
    virtual TScriptInterface<IIRrTarget> Copy() = 0;
    
    virtual TObjectPtr<class URnModel> Network() = 0;
};
