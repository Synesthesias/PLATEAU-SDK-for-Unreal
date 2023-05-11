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
#include "StaticMeshResources.h"
#include "PLATEAUTextureLoader.h"
#include "Materials/MaterialInstanceDynamic.h"

using namespace plateau::dataset;

namespace {
    FString GetIconFileName(const PredefinedCityModelPackage Package) {
        switch (Package) {
        case PredefinedCityModelPackage::Building: return "3dicon_Building.png";
        case PredefinedCityModelPackage::CityFurniture: return "3dicon_CityFurniture.png";
        case PredefinedCityModelPackage::Road: return "3dicon_Road.png";
        case PredefinedCityModelPackage::Vegetation: return "3dicon_Vegetation.png";
        default:
            UE_LOG(LogTemp, Error, TEXT("An icon for an unregistered package was requested."));
            return "";
        }
    }

    FString GetTexturePath(const FPLATEAUFeatureInfoMaterialKey& MaterialKey) {
        const FString WithTextDirName = "AreaIcon_WithText/";
        const FString NoTextDirName = "AreaIcon_NoText/";
        const TArray<FString> LodDirNames =
        { "LOD01/", "LOD01/", "LOD2/", "LOD3/" };

        FString Path = FPLATEAURuntimeModule::GetContentDir() + "/";
        Path += MaterialKey.bDetailed
            ? WithTextDirName
            : NoTextDirName;

        const auto Lod = FMath::Clamp(MaterialKey.Lod, 0, 3);
        Path += LodDirNames[Lod];

        Path += GetIconFileName(MaterialKey.Package);

        return Path;
    }

    UMaterialInstanceDynamic* CreateMaterial(UMaterial* BaseMaterial, const FPLATEAUFeatureInfoMaterialKey& MaterialKey) {
        const auto Material = UMaterialInstanceDynamic::Create(BaseMaterial, GetTransientPackage());
        const auto Texture = FPLATEAUTextureLoader::LoadTransient(GetTexturePath(MaterialKey));
        Material->SetTextureParameterValue(TEXT("Texture"), Texture);
        if (MaterialKey.bGrayout) {
            Material->SetScalarParameterValue(TEXT("Multiplier"), 0.2f);
            Material->SetScalarParameterValue(TEXT("Opacity"), 0.2f);
        } else {
            Material->SetScalarParameterValue(TEXT("Multiplier"), 0.7f);
            Material->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
        }
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
    const auto MeshCodes = MeshCode::getThirdMeshes(InExtent.GetNativeData());

    for (const auto& RawMeshCode : *MeshCodes) {
        // Panelが生成されていない場合は生成
        const auto MeshCode = UTF8_TO_TCHAR(RawMeshCode.get().c_str());
        if (!AsyncLoadedPanels.Find(MeshCode)) {
            const auto AsyncLoadedTile = MakeShared<FPLATEAUAsyncLoadedFeatureInfoPanel>(SharedThis(this), ViewportClient);
            AsyncLoadedTile->SetVisibility(Visibility);
            AsyncLoadedPanels.Add(MeshCode, AsyncLoadedTile);

            FPLATEAUFeatureInfoPanelInput Input;
            for (const auto& Package : GetDisplayedPackages()) {
                Input.Add(Package, FindGmlFiles(InDatasetAccessor, RawMeshCode, Package));
            }
            auto TileExtent = RawMeshCode.getExtent();
            const auto RawTileMax = GeoReference.GetData().project(TileExtent.max);
            const auto RawTileMin = GeoReference.GetData().project(TileExtent.min);
            FBox Box(FVector(RawTileMin.x, RawTileMin.y, RawTileMin.z),
                FVector(RawTileMax.x, RawTileMax.y, RawTileMax.z));
            AsyncLoadedTile->LoadAsync(Input, Box);

            continue;
        }

        // 範囲内のPanelについて表示をONにする
        const auto& AsyncLoadedPanel = AsyncLoadedPanels[MeshCode];
        AsyncLoadedPanel->Tick();
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
    for (const auto& Entry : AsyncLoadedPanels) {
        Entry.Value->SetVisibility(Visibility);
    }
}

TArray<PredefinedCityModelPackage> FPLATEAUFeatureInfoDisplay::GetDisplayedPackages()
{
    static TArray DisplayedPackages = {
        PredefinedCityModelPackage::Building,
        PredefinedCityModelPackage::Road,
        PredefinedCityModelPackage::CityFurniture,
        PredefinedCityModelPackage::Vegetation
    };

    return DisplayedPackages;
}

void FPLATEAUFeatureInfoDisplay::InitializeMaterials() {
    const auto BaseMaterial = Cast<UMaterial>(StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/PLATEAU-SDK-for-Unreal/FeatureInfoPanel_PanelIcon")));

    BackPanelMaterial = CreateBackPanelMaterial(BaseMaterial);

    // パッケージ、LOD等、地物情報アイコンを全種類列挙
    TArray<FPLATEAUFeatureInfoMaterialKey> Keys;
    for (const auto& Package : GetDisplayedPackages()) {
        for (int Lod = 0; Lod < 4; ++Lod) {
            Keys.Add({ Package, Lod, false, false });
            Keys.Add({ Package, Lod, false, true });
            Keys.Add({ Package, Lod, true, false });
            Keys.Add({ Package, Lod, true, true });
        }
    }

    // 全てのパターンについて地物情報アイコンのマテリアルを生成して保持
    for (const auto& Key : Keys) {
        FeatureInfoMaterials.Add(Key, CreateMaterial(BaseMaterial, Key));
    }
}
