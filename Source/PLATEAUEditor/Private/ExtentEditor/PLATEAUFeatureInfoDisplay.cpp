// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUFeatureInfoDisplay.h"
#include "PLATEAUGeometry.h"
#include "ExtentEditor/PLATEAUExtentEditorVPClient.h"
#include "ExtentEditor/PLATEAUAsyncLoadedFeatureInfoPanel.h"

#include <plateau/basemap/tile_projection.h>
#include <plateau/basemap/vector_tile_downloader.h>

#include <Async/Async.h>
#include <plateau/dataset/mesh_code.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "PLATEAURuntime.h"
#include "StaticMeshAttributes.h"
#include "PLATEAUTextureLoader.h"
#include "Materials/MaterialInstanceDynamic.h"

using namespace plateau::dataset;
using namespace UE::Tasks;

namespace {
    /**
     * @brief 最大並列数
     */
    constexpr int MaxParallelCount = 8;

     /**
      * @brief Packageに対応するアイコンのファイル名を取得します。
      */
    FString GetIconFileName(const PredefinedCityModelPackage Package) {
        switch (Package) {
        case PredefinedCityModelPackage::Building: return "building.png";
        case PredefinedCityModelPackage::Road: return "traffic.png";
        case PredefinedCityModelPackage::UrbanPlanningDecision: return "other.png";
        case PredefinedCityModelPackage::LandUse: return "other.png";
        case PredefinedCityModelPackage::CityFurniture: return "props.png";
        case PredefinedCityModelPackage::Vegetation: return "plants.png";
        case PredefinedCityModelPackage::Relief: return "terrain.png";
        case PredefinedCityModelPackage::DisasterRisk: return "other.png";
        case PredefinedCityModelPackage::Railway: return "traffic.png";
        case PredefinedCityModelPackage::Waterway: return "traffic.png";
        case PredefinedCityModelPackage::WaterBody: return "other.png";
        case PredefinedCityModelPackage::Bridge: return "bridge.png";
        case PredefinedCityModelPackage::Track: return "traffic.png";
        case PredefinedCityModelPackage::Square: return "traffic.png";
        case PredefinedCityModelPackage::Tunnel: return "bridge.png";
        case PredefinedCityModelPackage::UndergroundFacility: return "underground.png";
        case PredefinedCityModelPackage::UndergroundBuilding: return "underground.png";
        case PredefinedCityModelPackage::Area: return "other.png";
        case PredefinedCityModelPackage::OtherConstruction: return "other.png";
        case PredefinedCityModelPackage::Generic: return "other.png";
        case PredefinedCityModelPackage::Unknown: return "other.png";
        default:
            UE_LOG(LogTemp, Error, TEXT("An icon for an unregistered package was requested."));
            return "other.png";
        }
    }

    /**
     * @brief キーに対応するテクスチャのパスを取得します。
     * @return
     */
    FString GetTexturePath(const FPLATEAUFeatureInfoMaterialKey& MaterialKey) {
        const FString WithTextDirName = "AreaIcon_WithText/";
        const FString NoTextDirName = "AreaIcon_NoText/";
        const TArray<FString> LodDirNames = { "LOD01/", "LOD01/", "LOD2/", "LOD3/", "LOD4/" };

        FString Path = FPLATEAURuntimeModule::GetContentDir() + "/";
        Path += MaterialKey.bDetailed ? WithTextDirName : NoTextDirName;

        const auto Lod = FMath::Clamp(MaterialKey.Lod, 0, 4);
        Path += LodDirNames[Lod];
        Path += GetIconFileName(MaterialKey.Package);

        return Path;
    }

    UMaterialInstanceDynamic* CreateMaterial(UMaterial* BaseMaterial, const FPLATEAUFeatureInfoMaterialKey& MaterialKey) {
        const auto Material = UMaterialInstanceDynamic::Create(BaseMaterial, GetTransientPackage());
        const auto Texture = FPLATEAUTextureLoader::LoadTransient(GetTexturePath(MaterialKey));
        Material->SetTextureParameterValue(TEXT("Texture"), Texture);
        Material->SetScalarParameterValue(TEXT("Multiplier"), 1.0f);
        Material->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
        return Material;
    }

    UMaterialInstanceDynamic* CreateBackPanelMaterial(UMaterial* BaseMaterial) {
        const auto Texture = FPLATEAUTextureLoader::LoadTransient(FPLATEAURuntimeModule::GetContentDir() + "/round-button.png");
        const auto Material = UMaterialInstanceDynamic::Create(BaseMaterial, GetTransientPackage());
        Material->SetTextureParameterValue(TEXT("Texture"), Texture);
        Material->SetScalarParameterValue(TEXT("Multiplier"), 0.01f);
        Material->SetScalarParameterValue(TEXT("Opacity"), 0.6f);
        return Material;
    }

    std::shared_ptr<std::vector<GmlFile>> FindGmlFiles(
        const IDatasetAccessor& InDatasetAccessor,
        const MeshCode& InMeshCode,
        const PredefinedCityModelPackage InPackage) {

        return InDatasetAccessor
            .filterByMeshCodes({ InMeshCode })
            ->getGmlFiles(InPackage);
    }
}

FPLATEAUFeatureInfoDisplay::FPLATEAUFeatureInfoDisplay(
    const FPLATEAUGeoReference& InGeoReference,
    const TSharedPtr<FPLATEAUExtentEditorViewportClient> InViewportClient)
    : GeoReference(InGeoReference)
    , ViewportClient(InViewportClient) {
    InitializeMaterials();
}

FPLATEAUFeatureInfoDisplay::~FPLATEAUFeatureInfoDisplay() {}

void FPLATEAUFeatureInfoDisplay::UpdateAsync(const FPLATEAUExtent& InExtent, const IDatasetAccessor& InDatasetAccessor) {
    // 全てのパネルの状態を更新
    for (const auto& Entry: AsyncLoadedPanels) {
        Entry.Value->Tick();
    }

    const auto MeshCodes = MeshCode::getThirdMeshes(InExtent.GetNativeData());
    auto LoadingPanelCount = CountLoadingPanels();

    // 範囲内の各地域メッシュについてパネルの生成・読み込みを行う。
    for (const auto& RawMeshCode : *MeshCodes) {
        const FString MeshCode = UTF8_TO_TCHAR(RawMeshCode.get().c_str());

        // 最大並列数以上は同時に読み込まない。
        if (LoadingPanelCount >= MaxParallelCount)
            break;

        // 生成済みの場合はスキップ
        if (AsyncLoadedPanels.Find(MeshCode))
            continue;

        const auto AsyncLoadedTile = MakeShared<FPLATEAUAsyncLoadedFeatureInfoPanel>(SharedThis(this), ViewportClient);
        AsyncLoadedTile->SetVisibility(Visibility);
        AsyncLoadedPanels.Add(MeshCode, AsyncLoadedTile);

        FPLATEAUFeatureInfoPanelInput Input;
        const auto Packages = GetDisplayedPackages();

        for (const auto& Package : GetDisplayedPackages()) {
            Input.Add(Package, FindGmlFiles(InDatasetAccessor, RawMeshCode, Package));
        }
        const auto TileExtent = RawMeshCode.getExtent();
        const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
        const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
        const FBox Box{
            FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
            FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z)
        };

        AsyncLoadedTile->LoadMaxLodAsync(Input, Box);

        ++LoadingPanelCount;
    }
}

UMaterialInstanceDynamic* FPLATEAUFeatureInfoDisplay::GetFeatureInfoIconMaterial(
    const FPLATEAUFeatureInfoMaterialKey& Key) {
    if (!FeatureInfoMaterials.Find(Key))
        return nullptr;

    return FeatureInfoMaterials[Key];
}

UMaterialInstanceDynamic* FPLATEAUFeatureInfoDisplay::GetBackPanelMaterial() const {
    return BackPanelMaterial;
}

EPLATEAUFeatureInfoVisibility FPLATEAUFeatureInfoDisplay::GetVisibility() const {
    return Visibility;
}

void FPLATEAUFeatureInfoDisplay::SetVisibility(const EPLATEAUFeatureInfoVisibility Value) {
    Visibility = Value;
    for (const auto& Entry : AsyncLoadedPanels) {
        Entry.Value->SetVisibility(Visibility);
    }
}

TArray<PredefinedCityModelPackage> FPLATEAUFeatureInfoDisplay::GetDisplayedPackages() {
    static TArray DisplayedPackages = {
        PredefinedCityModelPackage::Building,
        PredefinedCityModelPackage::Road,
        PredefinedCityModelPackage::UrbanPlanningDecision,
        PredefinedCityModelPackage::LandUse,
        PredefinedCityModelPackage::CityFurniture,
        PredefinedCityModelPackage::Vegetation,
        PredefinedCityModelPackage::Relief,
        PredefinedCityModelPackage::DisasterRisk,
        PredefinedCityModelPackage::Railway,
        PredefinedCityModelPackage::Waterway,
        PredefinedCityModelPackage::WaterBody,
        PredefinedCityModelPackage::Bridge,
        PredefinedCityModelPackage::Track,
        PredefinedCityModelPackage::Square,
        PredefinedCityModelPackage::Tunnel,
        PredefinedCityModelPackage::UndergroundFacility,
        PredefinedCityModelPackage::UndergroundBuilding,
        PredefinedCityModelPackage::Area,
        PredefinedCityModelPackage::OtherConstruction,
        PredefinedCityModelPackage::Generic,
        PredefinedCityModelPackage::Unknown,
    };

    return DisplayedPackages;
}

void FPLATEAUFeatureInfoDisplay::InitializeMaterials() {
    const auto BaseMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PLATEAU-SDK-for-Unreal/FeatureInfoPanel_PanelIcon")));

    BackPanelMaterial = CreateBackPanelMaterial(BaseMaterial);

    // パッケージ、LOD等、地物情報アイコンを全種類列挙
    TArray<FPLATEAUFeatureInfoMaterialKey> Keys;
    for (const auto& Package : GetDisplayedPackages()) {
        for (int Lod = 0; Lod <= 4; ++Lod) {
            Keys.Add({ Package, Lod, false });
            Keys.Add({ Package, Lod, true });
        }
    }

    // 全てのパターンについて地物情報アイコンのマテリアルを生成して保持
    for (const auto& Key : Keys) {
        FeatureInfoMaterials.Add(Key, CreateMaterial(BaseMaterial, Key));
    }
}

int FPLATEAUFeatureInfoDisplay::CountLoadingPanels() {
    int Count = 0;
    for (const auto& Entry : AsyncLoadedPanels) {
        if (Entry.Value->GetStatus() == EPLATEAUFeatureInfoPanelStatus::Loading)
            ++Count;
    }
    return Count;
}
