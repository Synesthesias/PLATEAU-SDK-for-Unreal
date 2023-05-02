// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentFilter.generated.h"

class APLATEAUInstancedCityModel;


USTRUCT(BlueprintType)
struct FPLATEAUPackageLOD {
    GENERATED_BODY()

    FPLATEAUPackageLOD() {
    }

    FPLATEAUPackageLOD(const int InMinLod, const int InMaxLod) : MinLod(InMinLod), MaxLod(InMaxLod) {
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
    static int64 GetExistPackage(const APLATEAUInstancedCityModel* InSelection);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static TArray<FPLATEAUPackageLOD> GetPackageLODs(const APLATEAUInstancedCityModel* InSelection);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static int64 GetMaxCityObjectType();

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static void ApplyFilter(APLATEAUInstancedCityModel* InSelection, const int64 EnablePackage, const int MinLOD, const int MaxLOD, const bool bShowMultiLOD, const int64 EnableCityObject);
};
