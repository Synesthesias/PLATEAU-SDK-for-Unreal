// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "PLATEAUCityModel.h"
#include "PLATEAUInstancedCityModel.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "PLATEAUCityGmlProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadGmlCompleted, const FPLATEAUCityModel&, CityModel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadGmlFailed);

UCLASS()
class PLATEAURUNTIME_API UPLATEAUCityGmlProxy : public UBlueprintAsyncActionBase {
    GENERATED_BODY()

public:

    /** Execute the actual load */
    virtual void Activate() override;

    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "PLATEAU|CityGML", WorldContext = "WorldContextObject"))
        static UPLATEAUCityGmlProxy* LoadAsync(UObject* WorldContextObject, const FPLATEAUCityObjectInfo& GmlInfo);

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
        FOnLoadGmlCompleted Completed;

    // 非同期完了デリゲート
    UPROPERTY(BlueprintAssignable)
        FOnLoadGmlFailed Failed;

    static std::shared_ptr<const citygml::CityModel> Load(const FPLATEAUCityObjectInfo& GmlInfo);

private:
    const UObject* WorldContextObject;
    FPLATEAUCityObjectInfo GmlInfo;

    static TMap<FString, FPLATEAUCityModel> CityModelCache;
};
