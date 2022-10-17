// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CityGML/PLATEAUCityModel.h"
#include "PLATEAUGeometry.h"
#include "plateau/udx/udx_file_collection.h"
#include "plateau/polygon_mesh/mesh_extractor.h"
#include "plateau/polygon_mesh/mesh_extract_options.h"

#include "PLATEAUCityModelLoader.generated.h"

#define LOCTEXT_NAMESPACE "APLATEAUCityModelLoader"

namespace citygml {
    class CityModel;
}


UENUM(BlueprintType)
enum class ECityModelPackage : uint8 {
    Building,
    Road,
    Relief,
    UrbanFacility,
    Vegetation,
    Others
};

UENUM(BlueprintType)
enum class EBuildingTypeMask : uint8 {
    Door = 0,
    Window = 1,
    WallSurface,
    RoofSurface,
    GroundSurface,
    ClosureSurface,
    OuterFloorSurface,
    OuterCeilingSurface
};


USTRUCT()
struct FFeaturePlacementSettings {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        bool ShouldImport;

    UPROPERTY(EditAnywhere)
        int TargetLOD;
};

USTRUCT()
struct FCityModelPlacementSettings {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings BuildingPlacementSettings;

    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings RoadPlacementSettings;

    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings ReliefPlacementSettings;

    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings UrbanFacilityPlacementSettings;

    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings VegetationPlacementSettings;

    UPROPERTY(EditAnywhere)
        FFeaturePlacementSettings OtherPlacementSettings;

    FFeaturePlacementSettings& GetFeaturePlacementSettings(ECityModelPackage Package) {
        switch (Package) {
        case ECityModelPackage::Building: return BuildingPlacementSettings;
        case ECityModelPackage::Road: return RoadPlacementSettings;
        case ECityModelPackage::Relief: return ReliefPlacementSettings;
        case ECityModelPackage::UrbanFacility: return UrbanFacilityPlacementSettings;
        case ECityModelPackage::Vegetation: return VegetationPlacementSettings;
        default: return OtherPlacementSettings;
        }
    }

    // TODO: libplateau側に委譲
    static ECityModelPackage GetPackage(const FString& SubDirectoryName) {
        if (SubDirectoryName == FString(L"bldg"))
            return ECityModelPackage::Building;
        if (SubDirectoryName == FString(L"tran"))
            return ECityModelPackage::Road;
        if (SubDirectoryName == FString(L"dem"))
            return ECityModelPackage::Relief;
        return ECityModelPackage::Others;
    }

    static FText GetDisplayName(ECityModelPackage Package) {
        switch (Package) {
        case ECityModelPackage::Building: return LOCTEXT("Building", "建築物");
        case ECityModelPackage::Road: return LOCTEXT("Transportation", "道路");
        case ECityModelPackage::Relief: return LOCTEXT("Relief", "起伏");
        case ECityModelPackage::UrbanFacility: return LOCTEXT("UrbanFacility", "都市設備");
        case ECityModelPackage::Vegetation: return LOCTEXT("Vegetation", "植生");
        default: return LOCTEXT("Others", "その他(災害警戒区域等)");
        }
    }
};

UCLASS()
class PLATEAURUNTIME_API APLATEAUCityModelLoader : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLATEAUCityModelLoader();

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FString Source;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FPLATEAUExtent Extent;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FPLATEAUGeoReference GeoReference;

    UPROPERTY(EditAnywhere, Category = "CityModel")
        FCityModelPlacementSettings CityModelPlacementSettings;

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
        void Load();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

private:
    TMap<int, FPLATEAUCityModel> CityModelCache;
    void CreateRootComponent(AActor& Actor);
};

#undef LOCTEXT_NAMESPACE
