// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "Widgets/SCompoundWidget.h"

namespace plateau::dataset {
    enum class PredefinedCityModelPackage : uint32;
    class IDatasetAccessor;
}

/**
 *
 */
class SPLATEAUImportPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUImportPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle);

private:
    FString SourcePath;
    int ZoneID = 9;
    bool bImportFromServer = false;
    bool bIsSeletedServerPrefecture = false;
    int PrefectureID = 1;
    int MunicipalityID = 1;

    TWeakObjectPtr<APLATEAUCityModelLoader> CurrentLoader = nullptr;

    // 選択範囲の中心
    FVector3d ExtentCenter;
    // UI上で設定されるオフセット値
    FVector3d ReferencePoint;

    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;

    TSharedPtr<IDetailsView> ImportSettingsView = nullptr;
    TSharedPtr<class SPLATEAUServerDatasetSelectPanel> ServerPanelRef = nullptr;

    std::shared_ptr<plateau::dataset::IDatasetAccessor> ServerDatasetAccessor;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> LocalDatasetAccessor;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel();
    TSharedRef<SHorizontalBox> CreateFileSourceSelectButton();
    FReply OnBtnSelectFolderPathClicked();
};
