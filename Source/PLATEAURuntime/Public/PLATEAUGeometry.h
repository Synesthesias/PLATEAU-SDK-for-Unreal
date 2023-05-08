// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "plateau/geometry/geo_coordinate.h"
#include "plateau/geometry/geo_reference.h"

#include "PLATEAUGeometry.generated.h"

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUGeoCoordinate {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        double Latitude;
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        double Longitude;
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        double Height;

    FPLATEAUGeoCoordinate();
    FPLATEAUGeoCoordinate(const plateau::geometry::GeoCoordinate& InGeoCoordinate);

    plateau::geometry::GeoCoordinate GetNativeData() const;

    bool operator==(const FPLATEAUGeoCoordinate& other) const;
    bool operator!=(const FPLATEAUGeoCoordinate& other) const;

};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUExtent {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        FPLATEAUGeoCoordinate Min;
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        FPLATEAUGeoCoordinate Max;

    FPLATEAUExtent() = default;
    FPLATEAUExtent(const plateau::geometry::Extent& InExtent);

    plateau::geometry::Extent GetNativeData() const;
    bool operator==(const FPLATEAUExtent& other) const;
    bool operator!=(const FPLATEAUExtent& other) const;
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FPLATEAUGeoReference {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        int ZoneID = 9;
    UPROPERTY(EditAnywhere, Category = "PLATEAU|Geometry")
        FVector ReferencePoint = FVector::ZeroVector;

    FPLATEAUGeoReference();
    FPLATEAUGeoReference(const plateau::geometry::GeoReference& InGeoReference);

    plateau::geometry::GeoReference& GetData();
    void UpdateNativeData();

private:
    friend class UPLATEAUGeoReferenceBlueprintLibrary;

    plateau::geometry::GeoReference Data;
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
