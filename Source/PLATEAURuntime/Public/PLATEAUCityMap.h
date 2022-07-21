// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityMapMetadata.h"
#include "GameFramework/Actor.h"

#include "PLATEAUCityMap.generated.h"

UENUM(BlueprintType)
enum class EFeaturePlacementMode : uint8 {
    DontPlace,
    PlaceTargetLODOrLower,
    PlaceTargetLOD,
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
        EFeaturePlacementMode FeaturePlacementMode;

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
};

UCLASS()
class PLATEAURUNTIME_API APLATEAUCityMap : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLATEAUCityMap();


    UPROPERTY(EditAnywhere, Category = "CityModel")
        UCityMapMetadata* Metadata;

    UPROPERTY(EditAnywhere, Category = "CityModel")
        FCityModelPlacementSettings CityModelPlacementSettings;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
