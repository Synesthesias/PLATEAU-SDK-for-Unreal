#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include <plateau/network/client.h>

class SPLATEAUServerDatasetSelectPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUServerDatasetSelectPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    int PrefectureID = 0;
    int MunicipalityID = 0;
    bool bIsVisible = false;

    plateau::network::Client ClientRef;
    std::shared_ptr<std::vector<plateau::network::DatasetMetadataGroup>> DataSets;

    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;
    std::string datasetID;

    TMap<int, FText> PrefectureTexts;
    TMap<int, FText> MunicipalityTexts;
    TSharedPtr<SComboButton> MunicipalityComboButton;
    TSharedPtr<SEditableTextBox> ServerURL;
    const std::string DefaultServerURL = plateau::network::Client::getDefaultServerUrl();

    bool bLoadedClientData = false;
    bool bServerInitialized = false;
    FText MaxLODText;
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
            const auto& newDatasetID = DataSets->at(PrefectureID).datasets[MunicipalityID].id;
            if (newDatasetID != datasetID) {
                datasetID = newDatasetID;
                try {
                    const auto InDatasetSource = DatasetSource::createServer(newDatasetID, ClientRef);
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
    inline plateau::network::Client GetClientRef() {
        return ClientRef;
    }
    inline std::string GetServerDatasetID() {
        return DataSets->at(PrefectureID).datasets[MunicipalityID].id;
    }

private:
    void LoadClientData(const std::string& InServerURL);
    void LoadServerDataWithURL(const std::string& InServerURL);
    void InitUITexts();
    TSharedPtr<SVerticalBox> ConstructServerDataPanel();
    TSharedPtr<SVerticalBox> ConstructPrefectureSelectPanel();
    TSharedPtr<SVerticalBox> ConstructDatasetSelectPanel();
    TSharedPtr<SVerticalBox> ConstructDescriptionPanel();
};