// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

namespace plateau::dataset {
    class MeshCode;
    class GmlFile;
    class IDatasetAccessor;
    enum class PredefinedCityModelPackage : uint32;
}

enum class EPLATEAUFeatureInfoVisibility : uint8_t {
    Hidden = 0,
    Visible = 1,
    Detailed = 2,
};

/**
 * @brief 地物情報パネルのマテリアルを検索するためのキーです。
 */
struct FPLATEAUFeatureInfoMaterialKey {
    plateau::dataset::PredefinedCityModelPackage Package;
    int Lod;
    bool bGrayout;
    bool bDetailed;
};

inline bool operator==(const FPLATEAUFeatureInfoMaterialKey& A, const FPLATEAUFeatureInfoMaterialKey& B) {
    return A.Package == B.Package && A.Lod == B.Lod && A.bGrayout == B.bGrayout && A.bDetailed == B.bDetailed;
}

inline bool operator!=(const FPLATEAUFeatureInfoMaterialKey& A, const FPLATEAUFeatureInfoMaterialKey& B) {
    return A.Package != B.Package || A.Lod != B.Lod || A.bGrayout != B.bGrayout || A.bDetailed != B.bDetailed;
}

inline uint32 GetTypeHash(const FPLATEAUFeatureInfoMaterialKey& Key) {
    return
        FMath::FloorLog2(static_cast<uint32>(Key.Package)) // 0 ~ 31(5bits)
        + (Key.Lod << 6) // 0 ~ 4(3bits)
        + ((Key.bGrayout ? 1 : 0) << 10)
        + ((Key.bDetailed ? 1 : 0) << 11);
}

/**
 * 全ての地物情報パネルを管理します。UpdateAsyncを毎Tick呼び出す必要があります。
 */
class FPLATEAUFeatureInfoDisplay : public TSharedFromThis<FPLATEAUFeatureInfoDisplay> {
public:
    FPLATEAUFeatureInfoDisplay(const FPLATEAUGeoReference& InGeoReference, const TSharedPtr<class FPLATEAUExtentEditorViewportClient> InViewportClient);
    ~FPLATEAUFeatureInfoDisplay();

    void UpdateAsync(const FPLATEAUExtent& InExtent, const plateau::dataset::IDatasetAccessor& InDatasetAccessor);

    UMaterialInstanceDynamic* GetFeatureInfoIconMaterial(const FPLATEAUFeatureInfoMaterialKey& Key);
    UMaterialInstanceDynamic* GetBackPanelMaterial() const;

    /**
     * @brief 地物情報パネルの可視性を取得します。
     */
    EPLATEAUFeatureInfoVisibility GetVisibility() const;

    /**
     * @brief 地物情報パネルの可視性を設定します。
     */
    void SetVisibility(const EPLATEAUFeatureInfoVisibility Value);

    static TArray<plateau::dataset::PredefinedCityModelPackage> GetDisplayedPackages();

private:
    FPLATEAUGeoReference GeoReference;
    TWeakPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    EPLATEAUFeatureInfoVisibility Visibility;
    TMap<FString, TSharedPtr<class FPLATEAUAsyncLoadedFeatureInfoPanel>> AsyncLoadedPanels;
    TMap<FPLATEAUFeatureInfoMaterialKey, UMaterialInstanceDynamic*> FeatureInfoMaterials;
    UMaterialInstanceDynamic* BackPanelMaterial;

    void InitializeMaterials();
};
