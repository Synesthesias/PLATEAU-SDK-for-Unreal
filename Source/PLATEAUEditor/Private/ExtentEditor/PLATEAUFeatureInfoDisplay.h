#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

struct FPLATEAUExtent;

namespace plateau::dataset {
    class MeshCode;
    class GmlFile;
    class IDatasetAccessor;
    enum class PredefinedCityModelPackage : uint32;
}

struct FPLATEAUMeshCodeFeatureInfoInput {
    FString BldgGmlPath;
    int BldgMaxLod;
    FString RoadGmlPath;
    int RoadMaxLod;
    FString FrnGmlPath;
    int FrnMaxLod;
    FString VegGmlPath;
    int VegMaxLod;
};

/**
 * @brief 各メッシュコード(グリッド)に表示する地物情報のパネルを表します。
 */
struct FPLATEAUAsyncLoadedFeatureInfoPanel {
public:
    ~FPLATEAUAsyncLoadedFeatureInfoPanel() {
    }

    bool GetFullyLoaded() {
        FScopeLock Lock(&CriticalSection);
        return IsFullyLoaded;
    }

    USceneComponent* GetPanelComponent(int Index) {
        FScopeLock Lock(&CriticalSection);
        return PanelComponents[Index];
    }

    USceneComponent* GetDetailedPanelComponent(int Index) {
        FScopeLock Lock(&CriticalSection);
        return DetailedPanelComponents[Index];
    }

    USceneComponent* GetBackPanelComponent() {
        FScopeLock Lock(&CriticalSection);
        return BackPanelComponent;
    }

    void LoadAsync(const FPLATEAUMeshCodeFeatureInfoInput& Input);
    void LoadMaterial();
    void Tick();

private:
    const FString MakeTexturePath(const plateau::dataset::PredefinedCityModelPackage Type, const int LOD, const bool bEnableText);

private:
    FCriticalSection CriticalSection;
    std::atomic<bool> IsFullyLoaded;
    TArray<USceneComponent*> PanelComponents;
    TArray<USceneComponent*> DetailedPanelComponents;
    USceneComponent* BackPanelComponent;
    UMaterial* BaseMat;
    TFuture<FPLATEAUMeshCodeFeatureInfoInput> GetMaxLodTask;
};

/**
 * 全ての地物情報パネルを管理します。UpdateAsyncを毎Tick呼び出す必要があります。
 */
class FPLATEAUFeatureInfoDisplay {
public:
    FPLATEAUFeatureInfoDisplay(const FPLATEAUGeoReference& InGeoReference, const TSharedPtr<class FPLATEAUExtentEditorViewportClient> InViewportClient);
    ~FPLATEAUFeatureInfoDisplay();

    void UpdateAsync(const FPLATEAUExtent& InExtent, plateau::dataset::IDatasetAccessor& InDatasetAccessor, const bool bShow, const bool bDetailed);

private:
    FPLATEAUGeoReference GeoReference;
    TWeakPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    TMap<FString, TSharedPtr<FPLATEAUAsyncLoadedFeatureInfoPanel>> AsyncLoadedPanels;
    TSet<USceneComponent*> PanelsInScene;
};
