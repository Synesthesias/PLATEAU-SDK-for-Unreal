// Copyright 2023 Ministry of LandÅAInfrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PLATEAUGeometry.h"
#include "PLATEAUImportSettings.h"
#include <plateau/network/client.h>

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

USTRUCT()
struct FPLATEAUCityModelLoadStatus {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        int TotalGmlCount;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        int LoadedGmlCount;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        TArray<FString> LoadingGmls;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        TArray<FString> FailedGmls;
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

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        UPLATEAUImportSettings* ImportSettings;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        FPLATEAUCityModelLoadStatus Status;

    UPROPERTY(EditAnywhere, Category = "PLATEAU")
        bool bImportFromServer;

    std::shared_ptr<plateau::network::Client> ClientPtr;

    UFUNCTION(BlueprintCallable, Category = "PLATEAU")
        void LoadAsync();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};

#undef LOCTEXT_NAMESPACE
