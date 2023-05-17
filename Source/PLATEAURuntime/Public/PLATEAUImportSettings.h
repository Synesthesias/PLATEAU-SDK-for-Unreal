// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include <plateau/polygon_mesh/mesh_extract_options.h>

#include "CoreMinimal.h"

#include <plateau/dataset/city_model_package.h>

#include "PLATEAUImportSettings.generated.h"

#define LOCTEXT_NAMESPACE "PLATEAUImportSettings"

namespace plateau {
    namespace dataset {
        enum class PredefinedCityModelPackage : uint32;
    }
}

UENUM(BlueprintType)
enum class EPLATEAUMeshGranularity : uint8 {
    //! 最小地物単位(LOD2, LOD3の各部品)
    PerAtomicFeatureObject = 1,
    //! 主要地物単位(建築物、道路等)
    PerPrimaryFeatureObject = 0,
    //! 都市モデル地域単位(GMLファイル内のすべてを結合)
    PerCityModelArea = 2
};

USTRUCT()
struct PLATEAURUNTIME_API FPLATEAUFeatureImportSettings {
    GENERATED_USTRUCT_BODY()

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
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings Building;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings Road;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings Vegetation;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings CityFurniture;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings Relief;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings DisasterRisk;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings LandUse;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings UrbanPlanningDecision;
    UPROPERTY(EditAnywhere, Category = "Import Settings")
        FPLATEAUFeatureImportSettings Unknown;

    FPLATEAUFeatureImportSettings GetFeatureSettings(plateau::dataset::PredefinedCityModelPackage Package) const {
        switch (Package) {
        case plateau::dataset::PredefinedCityModelPackage::Building: return Building;
        case plateau::dataset::PredefinedCityModelPackage::Road: return Road;
        case plateau::dataset::PredefinedCityModelPackage::Vegetation: return Vegetation;
        case plateau::dataset::PredefinedCityModelPackage::CityFurniture: return CityFurniture;
        case plateau::dataset::PredefinedCityModelPackage::Relief: return Relief;
        case plateau::dataset::PredefinedCityModelPackage::DisasterRisk: return DisasterRisk;
        case plateau::dataset::PredefinedCityModelPackage::LandUse: return LandUse;
        case plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision: return UrbanPlanningDecision;
        case plateau::dataset::PredefinedCityModelPackage::Unknown: return Unknown;
        default: return Unknown;
        }
    }

    FPLATEAUFeatureImportSettings& GetFeatureSettingsRef(plateau::dataset::PredefinedCityModelPackage Package) {
        switch (Package) {
        case plateau::dataset::PredefinedCityModelPackage::Building: return Building;
        case plateau::dataset::PredefinedCityModelPackage::Road: return Road;
        case plateau::dataset::PredefinedCityModelPackage::Vegetation: return Vegetation;
        case plateau::dataset::PredefinedCityModelPackage::CityFurniture: return CityFurniture;
        case plateau::dataset::PredefinedCityModelPackage::Relief: return Relief;
        case plateau::dataset::PredefinedCityModelPackage::DisasterRisk: return DisasterRisk;
        case plateau::dataset::PredefinedCityModelPackage::LandUse: return LandUse;
        case plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision: return UrbanPlanningDecision;
        case plateau::dataset::PredefinedCityModelPackage::Unknown: return Unknown;
        default: return Unknown;
        }
    }
    
    static FName GetFeaturePlacementSettingsPropertyName(plateau::dataset::PredefinedCityModelPackage Package) {
        switch (Package) {
        case plateau::dataset::PredefinedCityModelPackage::Building: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Building);
        case plateau::dataset::PredefinedCityModelPackage::Road: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Road);
        case plateau::dataset::PredefinedCityModelPackage::Vegetation: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Vegetation);
        case plateau::dataset::PredefinedCityModelPackage::CityFurniture: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, CityFurniture);
        case plateau::dataset::PredefinedCityModelPackage::Relief: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Relief);
        case plateau::dataset::PredefinedCityModelPackage::DisasterRisk: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, DisasterRisk);
        case plateau::dataset::PredefinedCityModelPackage::LandUse: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, LandUse);
        case plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, UrbanPlanningDecision);
        case plateau::dataset::PredefinedCityModelPackage::Unknown: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Unknown);
        default: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Unknown);
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

    static TMap<EPLATEAUMeshGranularity, FText> GetGranularityTexts() {
        TMap<EPLATEAUMeshGranularity, FText> Items;
        Items.Add(EPLATEAUMeshGranularity::PerPrimaryFeatureObject, LOCTEXT("PrimaryFeatureObject", "主要地物単位"));
        Items.Add(EPLATEAUMeshGranularity::PerAtomicFeatureObject, LOCTEXT("AtomicFeatureObject", "最小地物単位"));
        Items.Add(EPLATEAUMeshGranularity::PerCityModelArea, LOCTEXT("CityModelArea", "地域単位"));
        return Items;
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

    static TMap<int64, FText> GetCategoryNames() {
        TMap<int64, FText> Items;
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building), LOCTEXT("CategoryBuilding", "建築物"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Road), LOCTEXT("CategoryRoad", "道路"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Vegetation), LOCTEXT("CategoryVegetation", "植生"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::CityFurniture), LOCTEXT("CategoryCityFurniture", "都市設備"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Relief), LOCTEXT("CategoryRelief", "起伏"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::DisasterRisk), LOCTEXT("CategoryDisasterRisk", "災害リスク"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::LandUse), LOCTEXT("CategoryLandUse", "土地利用"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision), LOCTEXT("CategoryUrbanPlanningDecision", "都市計画決定情報"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Unknown), LOCTEXT("CategoryUnknown", "その他"));
        return Items;
    }

    static TMap<int64, FText> GetFilteringNames() {
        TMap<int64, FText> Items;
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building), LOCTEXT("FilteringBuilding", "建造物(Building)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Road), LOCTEXT("FilteringRoad", "道路 (Road)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Vegetation), LOCTEXT("FilteringVegetation", "植生(Vegetation)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::CityFurniture), LOCTEXT("FilteringCityFurniture", "都市設備 (CityFurniture)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Relief), LOCTEXT("FilteringRelief", "土地起伏(Relief)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::DisasterRisk), LOCTEXT("FilteringDisasterRisk", "災害リスク (DisasterRisk)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::LandUse), LOCTEXT("FilteringLandUse", "土地利用 (LandUse)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision), LOCTEXT("FilteringUrbanPlanningDecision", "都市計画決定情報 (UrbanPlanningDecision)"));
        Items.Add(static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Unknown), LOCTEXT("FilteringUnknown", "その他 (Unknown)"));
        return Items;
    }
};

#undef LOCTEXT_NAMESPACE
