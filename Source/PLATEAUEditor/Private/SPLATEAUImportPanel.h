// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <plateau/io/mesh_convert_options.h>

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32;
}

struct FFeatureSettings {
    int MinLod;
    int MaxLod;
    bool IncludeAppearance;
    MeshGranularity Granularity;
    bool GenerateCollider;
};

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
    void Construct(const FArguments& InArgs);

private:
    FString SourcePath;
    int ZoneID = 9;
    TMap<plateau::udx::PredefinedCityModelPackage, FFeatureSettings> FeatureSettingsMap;

    TWeakPtr<SWindow> OwnerWindow;

    TSharedRef<SVerticalBox> CreateSourcePathSelectPanel();
    TSharedRef<SVerticalBox> CreateFeatureSettingsPanel(plateau::udx::PredefinedCityModelPackage Package);

    FReply OnBtnSelectGmlFileClicked();
};
