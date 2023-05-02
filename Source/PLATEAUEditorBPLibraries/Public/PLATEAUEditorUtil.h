// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUEditorUtil.generated.h"


UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUEditorUtil : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|Util")
    static TArray<int64> GetAllPackages();
};
