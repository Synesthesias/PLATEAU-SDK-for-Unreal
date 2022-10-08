// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "plateau/geometry/geo_coordinate.h"
#include "plateau/geometry/geo_reference.h"

#include "PLATEAUGeometry.generated.h"

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUGeoCoordinate {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        double Latitude;
    UPROPERTY(EditAnywhere)
        double Longitude;
    UPROPERTY(EditAnywhere)
        double Height;

    FPLATEAUGeoCoordinate();
    FPLATEAUGeoCoordinate(const plateau::geometry::GeoCoordinate& InGeoCoordinate);

    plateau::geometry::GeoCoordinate GetNativeData() const;
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUExtent {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        FPLATEAUGeoCoordinate Min;
    UPROPERTY(EditAnywhere)
        FPLATEAUGeoCoordinate Max;

    FPLATEAUExtent() = default;
    FPLATEAUExtent(plateau::geometry::Extent InExtent);

    plateau::geometry::Extent GetNativeData() const;
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUGeoReference {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        double ZoneID;
    UPROPERTY(EditAnywhere)
        FVector ReferencePoint;

    FPLATEAUGeoReference();
    FPLATEAUGeoReference(const plateau::geometry::GeoReference& InGeoReference);

    plateau::geometry::GeoReference& GetData();
    
private:
    friend class UPLATEAUGeoReferenceBlueprintLibrary;

    plateau::geometry::GeoReference Data;

    void UpdateNativeData();
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUGeoReferenceBlueprintLibrary : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        Category = "PLATEAU|Geometry")
        static FPLATEAUGeoCoordinate Unproject(
            UPARAM(ref) FPLATEAUGeoReference& GeoReference,
            const FVector& Point);

    UFUNCTION(
        BlueprintCallable,
        Category = "PLATEAU|Geometry")
        static FVector Project(
            UPARAM(ref) FPLATEAUGeoReference& GeoReference,
            const FPLATEAUGeoCoordinate& GeoCoordinate);
};
