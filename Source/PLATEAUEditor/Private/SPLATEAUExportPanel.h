// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/ISlateStyle.h"

class UPLATEAUExportSettings;

enum class EMeshFileFormat : uint8_t {
    OBJ = 0,
    FBX,
    GLTF,

    EMeshFileFormat_MAX,
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

    FString ExportPath = FPaths::ProjectContentDir();
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    EMeshFileFormat CurrentModelType = EMeshFileFormat::OBJ;
    TSharedPtr<IDetailsView> ExportSettingsView = nullptr;
    AActor* SelectingActor;
    bool bExportAsBinary = true;
    UPLATEAUExportSettings* Settings;
};
