// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RoadAdjust/PLATEAUReproducedRoad.h"
#include "RoadNetwork/Structure/RnModel.h"
#include "RoadNetwork/Structure/RnRoad.h"
#include "RoadNetwork/Structure/RnLane.h"
#include "PLATEAURrTarget.generated.h"

/**
 * 道路ネットワークから道路モデルを生成する処理において、どの部分を対象にするのかを記述します。
 * 対象が道路ネットワーク全体か、指定の道路かを切り替えます。
 * Rrは RoadReproducer の略です。
 */
UINTERFACE(MinimalAPI)
class UPLATEAURrTarget : public UInterface
{
    GENERATED_BODY()
};

class PLATEAURUNTIME_API IPLATEAURrTarget
{
    GENERATED_BODY()

public:
    virtual TArray<TRnRef_T<URnRoadBase>> GetRoadBases() const = 0;
    virtual TArray<TRnRef_T<URnRoad>> GetRoads() const = 0;
    virtual TArray<TRnRef_T<URnIntersection>> GetIntersections() const = 0;
    virtual const URnModel* GetNetwork() const = 0;
};

/**
 * IPLATEAURrTargetで、1つの道路モデル全体を対象に取ります。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAURrTargetModel : public UObject, public IPLATEAURrTarget
{
    GENERATED_BODY()

public:

    void Initialize(const TRnRef_T<URnModel> InNetwork);
    virtual TArray<TRnRef_T<URnRoadBase>> GetRoadBases() const override;
    virtual TArray<TRnRef_T<URnRoad>> GetRoads() const override;
    virtual TArray<TRnRef_T<URnIntersection>> GetIntersections() const override;
    virtual const URnModel* GetNetwork() const override;

private:
    TRnRef_T<URnModel> Model;
};

/**
 * IPLATEAURrTargetで、特定の道路または交差点を対象に取ります。
 */
UCLASS()
class PLATEAURUNTIME_API UPLATEAURrTargetRoadBases : public UObject, public IPLATEAURrTarget
{
    GENERATED_BODY()

public:
    void Initialize(const TRnRef_T<URnModel>& InNetwork, const TArray<TRnRef_T<URnRoadBase>>& InRoadBases);

    virtual TArray<TRnRef_T<URnRoadBase>> GetRoadBases() const override;
    virtual TArray<TRnRef_T<URnRoad>> GetRoads() const override;
    virtual TArray<TRnRef_T<URnIntersection>> GetIntersections() const override;
    virtual const URnModel* GetNetwork() const override;

private:
    TRnRef_T<URnModel> Network;
    TArray<TRnRef_T<URnRoadBase>> RoadBases;
};
