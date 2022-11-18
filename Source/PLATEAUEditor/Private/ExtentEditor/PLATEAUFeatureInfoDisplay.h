#pragma once

#include "CoreMinimal.h"

#include "PLATEAUGeometry.h"

struct FPLATEAUExtent;

namespace plateau::udx {
    class MeshCode;
    class GmlFileInfo;
    class UdxFileCollection;
}

struct FPLATEAUMeshCodeFeatureInfoInput {
    FString BldgGmlFile;
    FString RoadGmlFile;
    FString UrfGmlFile;
    FString VegGmlFile;
};

/**
 * @brief 各メッシュコード(グリッド)に表示する地物情報のパネルを表します。
 */
struct FPLATEAUAsyncLoadedFeatureInfoPanel {
public:
    bool GetFullyLoaded() {
        FScopeLock Lock(&CriticalSection);
        return IsFullyLoaded;
    }

    USceneComponent* GetComponent() {
        FScopeLock Lock(&CriticalSection);
        return PanelComponent;
    }

    void LoadAsync(const FPLATEAUMeshCodeFeatureInfoInput& Input);
private:
    FCriticalSection CriticalSection;
    bool IsFullyLoaded;
    USceneComponent* PanelComponent;
};

/**
 * 全ての地物情報パネルを管理します。UpdateAsyncを毎Tick呼び出す必要があります。
 */
class FPLATEAUFeatureInfoDisplay {
public:
    FPLATEAUFeatureInfoDisplay(const FPLATEAUGeoReference& InGeoReference, const TSharedPtr<class FPLATEAUExtentEditorViewportClient> InViewportClient);
    ~FPLATEAUFeatureInfoDisplay();

    void UpdateAsync(const FPLATEAUExtent& InExtent, const plateau::udx::UdxFileCollection& InFileCollection, const bool bShow);

private:
    FPLATEAUGeoReference GeoReference;
    TWeakPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient;

    TMap<FString, TSharedPtr<FPLATEAUAsyncLoadedFeatureInfoPanel>> AsyncLoadedPanels;
    TSet<USceneComponent*> PanelsInScene;
};
