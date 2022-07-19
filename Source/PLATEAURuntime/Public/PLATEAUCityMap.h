// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityMapMetadata.h"
#include "GameFramework/Actor.h"
#include "PLATEAUCityMap.generated.h"

UENUM(BlueprintType)
enum class EFeaturePlacementMode : uint8 {
    DontPlace,
    PlaceMaxLOD
};

UENUM(BlueprintType)
enum class EBuildingLOD3TypeMask : uint8 {
    Door,
    Window,
    WallSurface,
    RoofSurface,
    GroundSurface,
    ClosureSurface,
    OuterFloorSurface,
    OuterCeilingSurface
};

UENUM(BlueprintType)
enum class EBuildingLOD2TypeMask : uint8 {
    WallSurface,
    RoofSurface,
    GroundSurface,
    ClosureSurface,
    OuterFloorSurface,
    OuterCeilingSurface
};


USTRUCT()
struct FBuildingPlacementSettings {
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditAnywhere)
        EFeaturePlacementMode FeaturePlacementMode;

    UPROPERTY(EditAnywhere)
        int TargetLOD;

    UPROPERTY(EditAnywhere)
        EBuildingLOD3TypeMask BuildingLOD3TypeMask;

    UPROPERTY(EditAnywhere)
        EBuildingLOD2TypeMask BuildingLOD2TypeMask;
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

UCLASS()
class PLATEAURUNTIME_API APLATEAUCityMap : public AActor {
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    APLATEAUCityMap();


    UPROPERTY(EditAnywhere, Category = "CityModel")
        UCityMapMetadata* Metadata;

    UPROPERTY(EditAnywhere, Category = "CityModel")
        FBuildingPlacementSettings BuildingPlacementSettings;

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;

};
