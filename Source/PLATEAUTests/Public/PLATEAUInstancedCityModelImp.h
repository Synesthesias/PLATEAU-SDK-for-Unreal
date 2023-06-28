// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PLATEAUInstancedCityModelImp.generated.h"

class APLATEAUCityModelLoader;
struct FPackageInfoSettings;


USTRUCT(BlueprintType)
struct FGizmoData {
    GENERATED_BODY()

    FGizmoData(): MinX(0), MinY(0), MaxX(0), MaxY(0) {
    }

    FGizmoData(const double InMinX, const double InMinY, const double InMaxX, const double InMaxY): MinX(InMinX), MinY(InMinY), MaxX(InMaxX),
        MaxY(InMaxY) {
    }

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    double MinX;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    double MinY;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    double MaxX;

    UPROPERTY(BlueprintReadWrite, Category = "PLATEAU|Tests")
    double MaxY;
};

UCLASS()
class UPLATEAUInstancedCityModelImp : public UBlueprintFunctionLibrary {
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PLATEAU|Tests")
    static APLATEAUCityModelLoader* GetLocalCityModelLoader(const int ZoneId, const FVector& ReferencePoint, const int64 PackageMask,
                                                            const FString& SourcePath, const FGizmoData& GizmoData,
                                                            const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData);
};
