// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CityGML/PLATEAUCityModel.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUImportSettings.h"

#include "PLATEAUCityModelLoader.generated.h"

#define LOCTEXT_NAMESPACE "APLATEAUCityModelLoader"

namespace citygml {
    class CityModel;
}

class FPLATEAUMeshLoader;
enum class MeshGranularity;

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

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32_t;
}

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

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        UPLATEAUImportSettings* ImportSettings;

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
    void LoadAsync();
};

#undef LOCTEXT_NAMESPACE
