// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelAdjustmentBuilding.generated.h"


UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUModelAdjustmentBuilding : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static bool IsBuildingPackage(const int64 InPackage);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ModelAdjustmentPanel")
    static TArray<int64> GetAllBuildingSettingFlags();
};
