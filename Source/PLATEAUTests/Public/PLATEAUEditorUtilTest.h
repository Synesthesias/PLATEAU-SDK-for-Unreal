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

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static bool MakeDirectory(const FString& Path, const bool CreateTree);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static bool DeleteFile(const FString& Path);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static bool DeleteDirectory(const FString& Path);

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static TArray<FString> FindFiles(const FString& Path, const FString& Filter = "*");

    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|Util")
    static bool WriteToFile(const FString& Path, const FString& Text);
};
