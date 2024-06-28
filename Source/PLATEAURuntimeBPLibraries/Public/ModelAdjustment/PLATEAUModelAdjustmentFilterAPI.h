// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentFilterAPI.generated.h"

class APLATEAUInstancedCityModel;

USTRUCT(BlueprintType)
struct FPLATEAUPackageLod {
    GENERATED_BODY()

    FPLATEAUPackageLod() {
    }

    FPLATEAUPackageLod(const int InMinLod, const int InMaxLod) : MinLod(InMinLod), MaxLod(InMaxLod) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ModelAdjustmentAPI")
    int MinLod = 0;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|BPLibraries|ModelAdjustmentAPI")
    int MaxLod = 0;
};

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUModelAdjustmentFilterAPI : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static TMap<int64, FText> GetFilteringNames();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static int64 GetCityModelPackages(const APLATEAUInstancedCityModel* TargetCityModel);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static FPLATEAUPackageLod GetMinMaxLod(const APLATEAUInstancedCityModel* TargetCityModel, const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static void ApplyFilter(APLATEAUInstancedCityModel* TargetCityModel, const int64 EnablePackage, const TMap<int64, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bOnlyMaxLod, const int64 EnableCityObject);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static void FilterModel(APLATEAUInstancedCityModel* TargetCityModel, const TArray<EPLATEAUCityModelPackage> EnablePackages, const TMap<EPLATEAUCityModelPackage, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bOnlyMaxLod, const TArray<EPLATEAUCityObjectsType> EnableCityObjects);

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static TArray<EPLATEAUCityModelPackage> ConvertCityModelPackagesToEnumArray(const int64 Package);
};
