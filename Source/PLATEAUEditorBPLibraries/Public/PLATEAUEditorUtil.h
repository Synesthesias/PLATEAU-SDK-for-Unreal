// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUEditorUtil.generated.h"


UCLASS()
class PLATEAUEDITORBPLIBRARIES_API UPLATEAUEditorUtil : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
    static void* GetWindowHandle();
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|Util")
    static bool OpenDirectoryDialog(UPARAM(ref) FString& SourcePath);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|Util")
    static TArray<int64> GetAllPackages();
};
