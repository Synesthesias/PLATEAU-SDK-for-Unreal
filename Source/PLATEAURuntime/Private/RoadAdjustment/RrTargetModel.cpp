
// Fill out your copyright notice in the Description page of Project Settings.

#include "RoadAdjustment/RrTargetModel.h"
//#include "RoadAdjustment/RrRoadNetworkCopier.h"

URrTargetModel::URrTargetModel()
{
    Model = nullptr;
}

void URrTargetModel::Initialize(TObjectPtr<class URnModel> InModel)
{
    Model = InModel;
}

TArray<URnRoadBase*> URrTargetModel::RoadBases()
{
    TArray<URnRoadBase*> Result;
    
    // Roads()とIntersections()の結果を結合
    Result.Append(Roads());
    Result.Append(Intersections());
    
    return Result;
}

TArray<URnRoad*> URrTargetModel::Roads()
{
    TArray<URnRoad*> Result;
    if (Model)
    {
        Result.Append(Model->GetRoads());
    }
    return Result;
}

TArray<URnIntersection*> URrTargetModel::Intersections()
{
    TArray<URnIntersection*> Result;
    if (Model)
    {
        Result.Append(Model->GetIntersections());
    }
    return Result;
}

TScriptInterface<IIRrTarget> URrTargetModel::Copy()
{
    //URrRoadNetworkCopier* Copier = NewObject<URrRoadNetworkCopier>();
    //TMap<UObject*, UObject*> ObjectMapping;
    //URnModel* CopiedNetwork = Copier->Copy(Model, ObjectMapping);
    //
    //URrTargetModel* NewTarget = NewObject<URrTargetModel>();
    //NewTarget->Initialize(CopiedNetwork);
    //
    //return TScriptInterface<IIRrTarget>(NewTarget);
    return nullptr;
}

TObjectPtr<URnModel> URrTargetModel::Network()
{
    return Model;
}
