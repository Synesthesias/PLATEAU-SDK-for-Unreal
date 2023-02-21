// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
    TVec3d NativeReferencePoint;
    FPLATEAUGeoReference GeoReference;

    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;

    TSharedPtr<IDetailsView> ImportSettingsView = nullptr;
    TSharedPtr<class SPLATEAUServerDatasetSelectPanel> ServerPanelRef = nullptr;

    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel();
    TSharedRef<SHorizontalBox> CreateFileSourceSelectButton();
    FReply OnBtnSelectFolderPathClicked();
};
