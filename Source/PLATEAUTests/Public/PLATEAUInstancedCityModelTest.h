// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUInstancedCityModelTest.generated.h"

class APLATEAUCityModelLoader;
struct FPackageInfoSettings;


USTRUCT(BlueprintType)
struct FGizmoData {
    GENERATED_BODY()

    FGizmoData(): MinX(0), MinY(0), MaxX(0), MaxY(0) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests|ImportPanel")
    double MinX;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests|ImportPanel")
    double MinY;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests|ImportPanel")
    double MaxX;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests|ImportPanel")
    double MaxY;
};

UCLASS()
class UPLATEAUInstancedCityModelTest : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests|ImportPanel")
    static APLATEAUCityModelLoader* GetLocalCityModelLoader(const int ZoneID, const FVector& ReferencePoint, const int64 PackageMask, const FString& SourcePath, const FGizmoData& GizmoData, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData);
};
