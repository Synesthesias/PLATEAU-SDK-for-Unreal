// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUFilterReliefTest.generated.h"


UCLASS()
class UPLATEAUFilterReliefTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static bool IsReliefPackage(const int64 Package);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ModelAdjustmentPanel")
    static TArray<int64> GetAllReliefSettingFlags();
};
