#pragma once

#include <plateau/io/mesh_convert_options.h>

#include "CoreMinimal.h"

#include <plateau/udx/city_model_package.h>

#include "PLATEAUImportSettings.generated.h"

//namespace plateau::udx {
//    enum class PredefinedCityModelPackage : uint32;
//}

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

    FPLATEAUFeatureImportSettings GetFeatureSettings(plateau::udx::PredefinedCityModelPackage Package) const {
        switch (Package) {
        case plateau::udx::PredefinedCityModelPackage::Building: return Building;
        case plateau::udx::PredefinedCityModelPackage::Road: return Road;
        case plateau::udx::PredefinedCityModelPackage::Vegetation: Vegetation;
        case plateau::udx::PredefinedCityModelPackage::CityFurniture: return CityFurniture;
        case plateau::udx::PredefinedCityModelPackage::Relief: return Relief;
        case plateau::udx::PredefinedCityModelPackage::DisasterRisk: return DisasterRisk;
        case plateau::udx::PredefinedCityModelPackage::LandUse: return LandUse;
        case plateau::udx::PredefinedCityModelPackage::UrbanPlanningDecision: return UrbanPlanningDecision;
        case plateau::udx::PredefinedCityModelPackage::Unknown: return Unknown;
        default: return Unknown;
        }
    }

    static TArray<plateau::udx::PredefinedCityModelPackage> GetAllPackages() {
        return {
            plateau::udx::PredefinedCityModelPackage::Building,
            plateau::udx::PredefinedCityModelPackage::Road,
            plateau::udx::PredefinedCityModelPackage::Vegetation,
            plateau::udx::PredefinedCityModelPackage::CityFurniture,
            plateau::udx::PredefinedCityModelPackage::Relief,
            plateau::udx::PredefinedCityModelPackage::DisasterRisk,
            plateau::udx::PredefinedCityModelPackage::LandUse,
            plateau::udx::PredefinedCityModelPackage::UrbanPlanningDecision,
            plateau::udx::PredefinedCityModelPackage::Unknown,
        };
    }

    static MeshGranularity ConvertGranularity(EPLATEAUMeshGranularity Value) {
        // TODO: 共通化
        switch (Value) {
        case EPLATEAUMeshGranularity::PerPrimaryFeatureObject:
            return MeshGranularity::PerPrimaryFeatureObject;
        case EPLATEAUMeshGranularity::PerAtomicFeatureObject:
            return MeshGranularity::PerAtomicFeatureObject;
        case EPLATEAUMeshGranularity::PerCityModelArea:
            return MeshGranularity::PerCityModelArea;
        }

        return MeshGranularity::PerPrimaryFeatureObject;
    }
};