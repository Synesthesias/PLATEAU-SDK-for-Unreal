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
    TMap<int, FText> PrefectureTexts;
    TMap<int, FText> MunicipalityTexts;
    TSharedPtr<SComboButton> MunicipalityComboButton;
    bool bLoadedClientData = false;
    FText MaxLODText;
    FText DescriptionText;

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    void SetPanelVisibility(bool bVisible) { bIsVisible = bVisible; }
    void InitServerData();
    inline std::shared_ptr<plateau::dataset::IDatasetAccessor> GetDatasetAccessor() { return DatasetAccessor; }
    inline plateau::network::Client GetClientRef() { return ClientRef; }
    inline std::string GetServerDatasetID() { return DataSets->at(PrefectureID).datasets[MunicipalityID].id; }

private:
    void LoadClientData();
};