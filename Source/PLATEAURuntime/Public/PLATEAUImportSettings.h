#pragma once

#include "CoreMinimal.h"

#include "PLATEAUImportSettings.generated.h"

UENUM(BlueprintType)
enum class EPLATEAUMeshGranularity : uint8 {
    //! 最小地物単位(LOD2, LOD3の各部品)
    PerAtomicFeatureObject,
    //! 主要地物単位(建築物、道路等)
    PerPrimaryFeatureObject,
    //! 都市モデル地域単位(GMLファイル内のすべてを結合)
    PerCityModelArea
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUFeatureImportSettings {
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        bool bImport = true;

    UPROPERTY(EditAnywhere, Category = "Import Settings")
        bool bImportTexture = true;

    UPROPERTY(EditAnywhere, Category = "Import Settings")
        bool bSetCollider = true;

    UPROPERTY(EditAnywhere, Category = "Import Settings")
        EPLATEAUMeshGranularity MeshGranularity = EPLATEAUMeshGranularity::PerPrimaryFeatureObject;

    UPROPERTY(EditAnywhere, Category = "Import Settings", meta = (ClampMin = 0, UIMin = 0, ClamMax = 3, UIMax = 3))
        int MinLod = 0;

    UPROPERTY(EditAnywhere, Category = "Import Settings", meta = (ClampMin = 0, UIMin = 0, ClamMax = 3, UIMax = 3))
        int MaxLod = 3;
};

UCLASS()
class PLATEAURUNTIME_API UPLATEAUImportSettings : public UObject {
    GENERATED_BODY()
        
public:
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings Building;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings Road;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings Vegetation;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings CityFurniture;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings Relief;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings DisasterRisk;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings LandUse;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings UrbanPlanningDecision;
    UPROPERTY(EditAnywhere)
        FPLATEAUFeatureImportSettings Unknown;

    //FPLATEAUFeatureImportSettings& GetFeaturePlacementSettings(ECityModelPackage Package) {
    //    switch (Package) {
    //    case ECityModelPackage::Building: return BuildingPlacementSettings;
    //    case ECityModelPackage::Road: return RoadPlacementSettings;
    //    case ECityModelPackage::Relief: return ReliefPlacementSettings;
    //    case ECityModelPackage::UrbanFacility: return UrbanFacilityPlacementSettings;
    //    case ECityModelPackage::Vegetation: return VegetationPlacementSettings;
    //    default: return OtherPlacementSettings;
    //    }
    //}
};