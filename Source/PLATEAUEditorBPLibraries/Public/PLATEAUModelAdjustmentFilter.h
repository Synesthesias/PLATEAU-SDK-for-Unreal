// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentFilter.generated.h"

class APLATEAUInstancedCityModel;


USTRUCT(BlueprintType)
struct FPLATEAUPackageLod {
    GENERATED_BODY()

    FPLATEAUPackageLod() {
    }

    FPLATEAUPackageLod(const int InMinLod, const int InMaxLod) : MinLod(InMinLod), MaxLod(InMaxLod) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ModelAdjustmentPanel")
    int MinLod = 0;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ModelAdjustmentPanel")
    int MaxLod = 0;
};

UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUModelAdjustmentFilter : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static TMap<int64, FText> GetFilteringNames();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static int64 GetCityModelPackages(const APLATEAUInstancedCityModel* TargetCityModel);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static FPLATEAUPackageLod GetMinMaxLod(const APLATEAUInstancedCityModel* TargetCityModel, const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static void ApplyFilter(APLATEAUInstancedCityModel* TargetCityModel, const int64 EnablePackage, const TMap<int64, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bShowMultiLod, const int64 EnableCityObject);
};
