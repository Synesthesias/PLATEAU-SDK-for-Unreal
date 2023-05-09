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
    static int64 GetCityModelPackages(const APLATEAUInstancedCityModel* InSelection);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static FPLATEAUPackageLod GetMinMaxLod(const APLATEAUInstancedCityModel* InSelection, const int64 InPackage);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static int64 GetMaxCityObjectType();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static void ApplyFilter(APLATEAUInstancedCityModel* InSelection, const int64 EnablePackage, const int MinLOD, const int MaxLOD, const bool bShowMultiLOD, const int64 EnableCityObject);
};
