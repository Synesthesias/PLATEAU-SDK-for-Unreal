// Copyright 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include <plateau/polygon_mesh/mesh_extract_options.h>

#include "CoreMinimal.h"

#include <plateau/dataset/city_model_package.h>

#include "PLATEAUImportSettings.generated.h"

namespace plateau {
    namespace dataset {
        enum class PredefinedCityModelPackage : uint32;
    }
}

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

    FPLATEAUFeatureImportSettings GetFeatureSettings(plateau::dataset::PredefinedCityModelPackage Package) const {
        switch (Package) {
        case plateau::dataset::PredefinedCityModelPackage::Building: return Building;
        case plateau::dataset::PredefinedCityModelPackage::Road: return Road;
        case plateau::dataset::PredefinedCityModelPackage::Vegetation: Vegetation;
        case plateau::dataset::PredefinedCityModelPackage::CityFurniture: return CityFurniture;
        case plateau::dataset::PredefinedCityModelPackage::Relief: return Relief;
        case plateau::dataset::PredefinedCityModelPackage::DisasterRisk: return DisasterRisk;
        case plateau::dataset::PredefinedCityModelPackage::LandUse: return LandUse;
        case plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision: return UrbanPlanningDecision;
        case plateau::dataset::PredefinedCityModelPackage::Unknown: return Unknown;
        default: return Unknown;
        }
    }

    static TArray<plateau::dataset::PredefinedCityModelPackage> GetAllPackages() {
        return {
            plateau::dataset::PredefinedCityModelPackage::Building,
            plateau::dataset::PredefinedCityModelPackage::Road,
            plateau::dataset::PredefinedCityModelPackage::Vegetation,
            plateau::dataset::PredefinedCityModelPackage::CityFurniture,
            plateau::dataset::PredefinedCityModelPackage::Relief,
            plateau::dataset::PredefinedCityModelPackage::DisasterRisk,
            plateau::dataset::PredefinedCityModelPackage::LandUse,
            plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision,
            plateau::dataset::PredefinedCityModelPackage::Unknown,
        };
    }

    static plateau::polygonMesh::MeshGranularity ConvertGranularity(EPLATEAUMeshGranularity Value) {
        // TODO: 共通化
        switch (Value) {
        case EPLATEAUMeshGranularity::PerPrimaryFeatureObject:
            return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
        case EPLATEAUMeshGranularity::PerAtomicFeatureObject:
            return plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
        case EPLATEAUMeshGranularity::PerCityModelArea:
            return plateau::polygonMesh::MeshGranularity::PerCityModelArea;
        }

        return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
    }
};