// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUImportModelBtn.h"

#include <plateau/dataset/dataset_source.h>
#include "AssetSelection.h"
#include "PLATEAUCityModelLoader.h"
#include "PLATEAUEditor/Public/PLATEAUEditor.h"
#include "PLATEAUEditor/Public/ExtentEditor/PLATEAUExtentEditor.h"

/**
 * @brief モデルインポート開始
 * @param ZoneID ゾーンID
 * @param ReferencePoint リファレンス位置
 * @param PackageInfoSettingsData UIで設定されたパッケージ設定情報データ
 * @param bImportFromServer サーバーからインポートするか？
 */
void UPLATEAUImportModelBtn::ImportModel(const int ZoneID, const FVector& ReferencePoint, const TMap<int64, FPackageInfoSettings>& PackageInfoSettingsData, const bool bImportFromServer) {
    const auto& ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();
    const auto EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
    const auto EmptyActorAsset = EmptyActorAssetData.GetAsset();
    const auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
    const auto Loader = Cast<APLATEAUCityModelLoader>(Actor);
    Loader->bImportFromServer = bImportFromServer;

    if (bImportFromServer) {
        Loader->ClientPtr = ExtentEditor->GetClientPtr();
        Loader->Source = ExtentEditor->GetServerDatasetID().c_str();
    } else {
        // ClientPtrは何か設定しないとクラッシュします
        Loader->ClientPtr = std::make_shared<plateau::network::Client>("", "");
        Loader->Source = ExtentEditor->GetSourcePath();
    }

    const auto& Extent = IPLATEAUEditorModule::Get().GetExtentEditor()->GetExtent();
    Loader->Extent = Extent.GetValue();
    Loader->Extent.Min.Height = -100000;
    Loader->Extent.Max.Height = 100000;

    Loader->GeoReference.ReferencePoint = ReferencePoint;
    Loader->GeoReference.ZoneID = ZoneID;
    Loader->GeoReference.UpdateNativeData();

    const auto& PackageMask = bImportFromServer ? ExtentEditor->GetServerPackageMask() : ExtentEditor->GetLocalPackageMask();
    const auto ImportSettings = DuplicateObject(GetMutableDefault<UPLATEAUImportSettings>(), Loader);
    for (const auto& Package : UPLATEAUImportSettings::GetAllPackages()) {
        if ((Package & PackageMask) == plateau::dataset::PredefinedCityModelPackage::None) continue;

        const auto& PackageInfoSettings = PackageInfoSettingsData[static_cast<int64>(Package)];
        auto& Feature = ImportSettings->GetFeatureSettingsRef(Package);
        Feature.bImport = PackageInfoSettings.bImport;
        Feature.bImportTexture = PackageInfoSettings.bTextureImport;
        Feature.MeshGranularity = static_cast<EPLATEAUMeshGranularity>(PackageInfoSettings.Granularity);
        Feature.MinLod = PackageInfoSettings.MinLod;
        Feature.MaxLod = PackageInfoSettings.MaxLod;
    }

    Loader->ImportSettings = ImportSettings;
    Loader->LoadAsync();
}
