// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32;
    class UdxFileCollection;
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

    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;

    TSharedPtr<IDetailsView> ImportSettingsView = nullptr;

    std::shared_ptr<plateau::udx::UdxFileCollection> FileCollection;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel();
    TSharedRef<SVerticalBox> CreateServerPrefectureSelectPanel();
    TSharedRef<SHorizontalBox> CreateFileSourceSelectButton();
    FReply OnBtnSelectFolderPathClicked();
};
