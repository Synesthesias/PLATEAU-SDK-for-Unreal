// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportLocalBtn.generated.h"


UCLASS()
class UPLATEAUImportLocalBtn : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
    static void* GetWindowHandle();
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static bool OpenDirectoryDialog(UPARAM(ref) bool& IsDatasetValid, UPARAM(ref) FString& SourcePath);	
};
