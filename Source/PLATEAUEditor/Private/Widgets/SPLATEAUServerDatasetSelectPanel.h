// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

#include <plateau/network/client.h>

class UPLATEAUServerConnectionSettings;

/**
 * @brief UIに表示するデータセット情報
 */
struct FPLATEAUServerDatasetMetadata {
    FString Title;
    FString Description;
    std::string ID;
};

class SPLATEAUServerDatasetSelectPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUServerDatasetSelectPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    void SetPanelVisibility(bool bVisible) {
        bIsVisible = bVisible;
    }

    std::string GetServerDatasetID() const;

    std::shared_ptr<plateau::network::Client> GetClientPtr() {
        return ClientPtr;
    }

    void SetSelectDatasetCallback(const TFunction<void()> Function) {
        OnSelectDataset = Function;
    }

    //virtual void Tick(FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    UPLATEAUServerConnectionSettings* Settings;
    TSharedPtr<IDetailsView> ConnectionSettingsView;
    TFunction<void()> OnSelectDataset;
    TFuture<void> GetDatasetMetadataTask;

    bool bIsVisible = false;

    std::atomic<bool> bIsNativeDatasetMetadataAvailable = false;
    std::atomic<bool> bIsGettingNativeDatasetMetadata = false;
    bool bHasDatasetMetadataLoaded = false;

    std::shared_ptr<plateau::network::Client> ClientPtr;
    std::shared_ptr<std::vector<plateau::network::DatasetMetadataGroup>> NativeDatasetMetadataGroups;

    TMap<FString, TArray<FPLATEAUServerDatasetMetadata>> DatasetMetadataByGroup;

    FString SelectedGroupTitle;
    FPLATEAUServerDatasetMetadata SelectedDataset;

    FCriticalSection GetDatasetMetadataSection;

    TFuture<void> GetDatasetMetadataAsync(const std::string& InServerURL = "", const std::string& InToken = "");
    void LoadDatasetMetadataFromNative();

    TSharedPtr<SVerticalBox> ConstructServerDataPanel();
    TSharedPtr<SHorizontalBox> ConstructDatasetGroupSelectPanel();
    TSharedPtr<SHorizontalBox> ConstructDatasetSelectPanel();
    TSharedPtr<SVerticalBox> ConstructDescriptionPanel();

    // Callbacks
    FText OnGetDatasetGroupText() const;
    FText OnGetDatasetText() const;
    TSharedRef<SWidget> OnGetDatasetGroupMenuContent();
    TSharedRef<SWidget> OnGetDatasetMenuContent();
    FText OnGetDescriptionText() const;
};