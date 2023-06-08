// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUEditorUtilTest.generated.h"


UCLASS()
class UPLATEAUEditorUtilTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static TArray<int64> GetAllPackages();
};
