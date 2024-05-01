// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUModelLandscapeAPI.generated.h"

class APLATEAUInstancedCityModel;

UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAUModelLandscapeAPI : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|CreateLandscape")
    static void CreateLandscape(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, bool bDestroyOriginal, FPLATEAULandscapeParam Param);

    UFUNCTION(BlueprintCallable, meta = (WorldContext = "Context"), Category = "PLATEAU|BPLibraries|CreateLandscapeDummy")
    static void CreateLandscapeDummy(const UObject* Context);

};
