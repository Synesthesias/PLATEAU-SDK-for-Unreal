// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/ISlateStyle.h"

enum class EExportFileFormat : uint8_t {
    OBJ = 0,
    FBX,
    GLTF,

    EExportFileFormat_MAX,
};

class SPLATEAUExportPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUExportPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

public:
    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle);

private:
    TSharedRef<SVerticalBox> CreateExportPathSelectPanel();
    FReply OnBtnSelectGmlFileClicked();

    FString ExportPath;
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    EExportFileFormat CurrentModelType = EExportFileFormat::OBJ;
    TSharedPtr<IDetailsView> ExportSettingsView = nullptr;
};
