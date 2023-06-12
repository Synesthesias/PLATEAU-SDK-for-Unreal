// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUFilterTest.generated.h"

class APLATEAUInstancedCityModel;
struct FPLATEAUPackageLod;


UCLASS()
class UPLATEAUFilterTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static int64 GetCityModelPackages(const APLATEAUInstancedCityModel* TargetCityModel);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static FPLATEAUPackageLod GetMinMaxLod(const APLATEAUInstancedCityModel* TargetCityModel, const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static void ApplyFilter(APLATEAUInstancedCityModel* TargetCityModel, const int64 EnablePackage, const TMap<int64, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bShowMultiLOD, const int64 EnableCityObject);
};
