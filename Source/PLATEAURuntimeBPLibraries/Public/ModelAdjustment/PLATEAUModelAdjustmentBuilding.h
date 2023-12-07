// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentBuilding.generated.h"


UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUModelAdjustmentBuilding : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static bool IsBuildingPackage(const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentAPI")
    static TArray<int64> GetAllBuildingSettingFlags();
};
