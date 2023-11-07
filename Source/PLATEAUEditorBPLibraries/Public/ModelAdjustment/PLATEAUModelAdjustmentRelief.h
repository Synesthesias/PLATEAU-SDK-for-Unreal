// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentRelief.generated.h"


UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUModelAdjustmentRelief : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static bool IsReliefPackage(const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static TArray<int64> GetAllReliefSettingFlags();
};
