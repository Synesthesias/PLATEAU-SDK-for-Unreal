#pragma once

#include <plateau/dataset/dataset_source.h>
#include <plateau/dataset/i_dataset_accessor.h>

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include <plateau/network/client.h>

class UPLATEAUServerConnectionSettings;

class SPLATEAUServerDatasetSelectPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUServerDatasetSelectPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    UPLATEAUServerConnectionSettings* Settings;
    TSharedPtr<IDetailsView> ConnectionSettingsView;

    int PrefectureID = 0;
    int MunicipalityID = 0;
    bool bIsVisible = false;

    std::shared_ptr<plateau::network::Client> ClientPtr;
    std::shared_ptr<std::vector<plateau::network::DatasetMetadataGroup>> DataSets;

    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
    std::string datasetID;

    TMap<int, FText> PrefectureTexts;
    TMap<int, FText> MunicipalityTexts;
    TSharedPtr<SComboButton> MunicipalityComboButton;

    bool bLoadedClientData = false;
    bool bServerInitialized = false;
    FText DescriptionText;
    FCriticalSection Mutex;

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    void SetPanelVisibility(bool bVisible) {
        bIsVisible = bVisible;
    }
    void InitServerData();
    inline std::shared_ptr<plateau::dataset::IDatasetAccessor> GetDatasetAccessor() {
        if (bLoadedClientData) {
            if (DataSets->size() <= PrefectureID)
                return nullptr;

            if (DataSets->at(PrefectureID).datasets.size() < MunicipalityID)
                return nullptr;

            const auto& newDatasetID = DataSets->at(PrefectureID).datasets[MunicipalityID].id;
            if (newDatasetID != datasetID) {
                datasetID = newDatasetID;
                try {
                    const auto InDatasetSource = plateau::dataset::DatasetSource::createServer(newDatasetID, *ClientPtr);
                    DatasetAccessor = InDatasetSource.getAccessor();
                }
                catch (...) {
                    DatasetAccessor = nullptr;
                    UE_LOG(LogTemp, Error, TEXT("Invalid Server Dataset ID"));
                }
            }
            return DatasetAccessor;
        } else {
            return nullptr;
        }
    }
    inline std::shared_ptr<plateau::network::Client> GetClientPtr() {
        return ClientPtr;
    }
    inline std::string GetServerDatasetID() {
        return DataSets->at(PrefectureID).datasets[MunicipalityID].id;
    }

private:
    void LoadClientData(const std::string& InServerURL = "", const std::string& InToken = "");
    void InitUITexts();
    TSharedPtr<SVerticalBox> ConstructServerDataPanel();
    TSharedPtr<SVerticalBox> ConstructPrefectureSelectPanel();
    TSharedPtr<SVerticalBox> ConstructDatasetSelectPanel();
    TSharedPtr<SVerticalBox> ConstructDescriptionPanel();
};