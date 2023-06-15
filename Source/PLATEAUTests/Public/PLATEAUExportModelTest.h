// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUExportModelTest.generated.h"

class APLATEAUInstancedCityModel;


UCLASS()
class UPLATEAUExportModelTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ExportPanel")
    static void ExportModel(APLATEAUInstancedCityModel* TargetCityModel, const FString& ExportPath, const uint8 FileFormat, const bool bExportAsBinary, const bool bExportHiddenModel, const bool bExportTexture, const uint8 CoordinateSystem, const uint8 TransformType);
};
