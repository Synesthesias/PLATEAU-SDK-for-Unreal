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

enum class EPLATEAUFeatureInfoVisibility : uint8_t;

typedef TMap<plateau::dataset::PredefinedCityModelPackage, std::shared_ptr<std::vector<plateau::dataset::GmlFile>>> FPLATEAUFeatureInfoPanelInput;

/**
 * @brief 各メッシュコード(グリッド)に表示する地物情報のパネルを表します。
 */
class FPLATEAUAsyncLoadedFeatureInfoPanel{
public:
    explicit FPLATEAUAsyncLoadedFeatureInfoPanel(
        const TWeakPtr<class FPLATEAUFeatureInfoDisplay> Owner,
        const TWeakPtr<class FPLATEAUExtentEditorViewportClient> ViewportClient);
    ~FPLATEAUAsyncLoadedFeatureInfoPanel() {}

    bool GetFullyLoaded() {
        return bFullyLoaded;
    }

    /**
     * @brief 地物情報パネルの可視性を取得します。
     */
    EPLATEAUFeatureInfoVisibility GetVisibility() const;

    /**
     * @brief 地物情報パネルの可視性を設定します。
     */
    void SetVisibility(const EPLATEAUFeatureInfoVisibility Value);

    /**
     * @brief GMLファイルの一覧を入力として、非同期に地物情報を読み込みます。
     * パネルの可視化は読み込みが完了した後にTickが呼び出された際に行われます。
     *
     * @param Input GMLファイルの一覧
     * @param InBox パネルの表示範囲
     */
    void LoadAsync(const FPLATEAUFeatureInfoPanelInput& Input, const FBox& InBox);
    void Tick();

private:
    TWeakPtr<FPLATEAUFeatureInfoDisplay> Owner;
    TWeakPtr<FPLATEAUExtentEditorViewportClient> ViewportClient;
    
    TAtomic<bool> bFullyLoaded;
    TFuture<TMap<plateau::dataset::PredefinedCityModelPackage, int>> GetMaxLodTask;

    EPLATEAUFeatureInfoVisibility Visibility;
    FBox Box;
    TArray<USceneComponent*> IconComponents;
    TArray<USceneComponent*> DetailedIconComponents;
    USceneComponent* BackPanelComponent;

    void ApplyVisibility() const;
};
