// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUFilterBuildingTest.generated.h"


UCLASS()
class UPLATEAUFilterBuildingTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static bool IsBuildingPackage(const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static TArray<int64> GetAllBuildingSettingFlags();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static int64 GetBuildingPackage();
};
