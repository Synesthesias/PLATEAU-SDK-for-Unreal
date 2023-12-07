// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAURuntimeUtil.generated.h"


UCLASS()
class PLATEAURUNTIMEBPLIBRARIES_API UPLATEAURuntimeUtil : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|Util")
    static TArray<int64> GetAllPackages();

    UFUNCTION(BlueprintCallable, Category = "PLATEAU|BPLibraries|Util")
    static TArray<EPLATEAUCityModelPackage> GetAllCityModelPackages();
};
