// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUImportAreaSelectBtn.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUImportModelBtn.generated.h"


UCLASS()
class UPLATEAUImportModelBtn : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|BPLibraries|ImportPanel")
    static void ImportModel(const int ZoneID, const FVector& ReferencePoint, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData, const bool bImportFromServer);
};