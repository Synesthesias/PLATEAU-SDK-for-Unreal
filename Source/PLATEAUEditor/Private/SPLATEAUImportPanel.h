// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <plateau/io/mesh_convert_options.h>

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/ISlateStyle.h"

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32;
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

    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;

    TSharedPtr<IDetailsView> BuildingImportSettingsView = nullptr;
    TSharedPtr<IDetailsView> RoadImportSettingsView = nullptr;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel();
    FReply OnBtnSelectGmlFileClicked();
};
