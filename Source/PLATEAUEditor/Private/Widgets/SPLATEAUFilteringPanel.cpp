// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "SPLATEAUFilteringPanel.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SHeader.h"
#include "PLATEAUEditorStyle.h"
#include "Engine/Selection.h"
#include "PLATEAUInstancedCityModel.h"
#include "Widgets/Input/SNumericEntryBox.h"

#define LOCTEXT_NAMESPACE "SPLATEAUFilteringPanel"
#define PANEL_ENABLE_TEXT_COLOR FSlateColor(FColor(128, 128, 128, 255))
#define PANEL_DISABLE_TEXT_COLOR FSlateColor(FColor(30, 30, 30, 255))

using namespace plateau::dataset;
using namespace citygml;

void SPLATEAUFilteringPanel::SetAllFlags(const bool InBool) {
    if (InBool) {
        PredefinedCityModelPackage tmp = PredefinedCityModelPackage::Building | PredefinedCityModelPackage::CityFurniture |
            PredefinedCityModelPackage::DisasterRisk |PredefinedCityModelPackage::LandUse | 
            PredefinedCityModelPackage::Relief | PredefinedCityModelPackage::Road | 
            PredefinedCityModelPackage::Unknown | PredefinedCityModelPackage::UrbanPlanningDecision | PredefinedCityModelPackage::Vegetation;
        EnablePackage = tmp;
        EnableCityObjects = citygml::CityObject::CityObjectsType::COT_All;
    }
    else {
        EnablePackage = PredefinedCityModelPackage::None;
        EnableCityObjects = (citygml::CityObject::CityObjectsType)0;
    }
}

void SPLATEAUFilteringPanel::SetAllBuildingSettingFlag(const bool InBool) {
    CityObject::CityObjectsType tmp = CityObject::CityObjectsType::COT_BuildingInstallation | CityObject::CityObjectsType::COT_Door |
        CityObject::CityObjectsType::COT_Window | CityObject::CityObjectsType::COT_BuildingPart |
        CityObject::CityObjectsType::COT_WallSurface | CityObject::CityObjectsType::COT_RoofSurface |
        CityObject::CityObjectsType::COT_GroundSurface | CityObject::CityObjectsType::COT_ClosureSurface |
        CityObject::CityObjectsType::COT_OuterCeilingSurface | CityObject::CityObjectsType::COT_OuterFloorSurface;
    if (InBool) {
        EnableCityObjects = EnableCityObjects | tmp;
    }
    else {
        EnableCityObjects = EnableCityObjects & ~tmp;
    }
}
void SPLATEAUFilteringPanel::SetAllReliefSettingFlag(const bool InBool) {
    CityObject::CityObjectsType tmp = CityObject::CityObjectsType::COT_TINRelief | CityObject::CityObjectsType::COT_MassPointRelief;
    if (InBool) {
        EnableCityObjects = EnableCityObjects | tmp;
    }
    else {
        EnableCityObjects = EnableCityObjects & ~tmp;
    }
}

void SPLATEAUFilteringPanel::SetAllVegetationSettingFlag(const bool InBool) {
    CityObject::CityObjectsType tmp = CityObject::CityObjectsType::COT_SolitaryVegetationObject | CityObject::CityObjectsType::COT_PlantCover;
    if (InBool) {
        EnableCityObjects = EnableCityObjects | tmp;
    }
    else {
        EnableCityObjects = EnableCityObjects & ~tmp;
    }
}

void SPLATEAUFilteringPanel::CheckTargetLODs() {
    if (bActorSelected == false) {
        AvailablePackage = SelectingActor->GetExistPackage();
        TargetLODs = SelectingActor->GetPackageLODs();
        TargetMaxLOD = TargetLODs[0].MaxLOD;
        TargetMinLOD = TargetLODs[0].MinLOD;
    }
}

void SPLATEAUFilteringPanel::SetEnableStatFromGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage, const bool InBool) {
    if (InBool)
        EnablePackage = EnablePackage | TargetPackage;
    else
        EnablePackage = (PredefinedCityModelPackage)((uint32_t)EnablePackage & ~(uint32_t)(TargetPackage));
}

bool SPLATEAUFilteringPanel::GetEnableStatFromGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage) {
    return ((uint32_t)EnablePackage & (uint32_t)TargetPackage);
}

void SPLATEAUFilteringPanel::SetMaxBuildingLOD(const int InLOD) {
    if (TargetLODs[0].MinLOD <= InLOD && InLOD <= TargetLODs[0].MaxLOD) {
        if (InLOD < TargetMinLOD) {
            TargetMaxLOD = TargetMinLOD = InLOD;
        }
        else {
            TargetMaxLOD = InLOD;
        }
    }
}

void SPLATEAUFilteringPanel::SetMinBuildingLOD(const int InLOD) {
    if ((TargetLODs[0].MinLOD <= InLOD) && (InLOD <= TargetLODs[0].MaxLOD)) {
        if (TargetMaxLOD < InLOD) {
            TargetMaxLOD = TargetMinLOD = InLOD;
        }
        else {
            TargetMinLOD = InLOD;
        }
    }
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUFilteringPanel::Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;
    ChildSlot[
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructSelectingActorPanel().ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructHeader().ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            ConstructBuilldingPanel().ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 5, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::CityFurniture, (LOCTEXT("CityFurniture", " 都市設備 (CityFurniture)"))).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::UrbanPlanningDecision, LOCTEXT("CityObjectGroup", " 都市計画決定情報 (UrbanPlanningDecision)")).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::LandUse, LOCTEXT("LandUse", " 土地利用 (LandUse)")).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            ConstructReliefPanel().ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 5, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::Road, LOCTEXT("Road", " 道路 (Road)")).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            ConstructVegetationPanel().ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 5, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::DisasterRisk, LOCTEXT("DisasterRisk", " 災害リスク (DisasterRisk)")).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            ConstructGenericPanel(PredefinedCityModelPackage::Unknown, LOCTEXT("Unknown", " その他 (Unknown)")).ToSharedRef()
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 15, 0, 30)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.1f)

            + SHorizontalBox::Slot()
            .FillWidth(0.8f)
            [
                SNew(SButton)
                .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("FilteringButton", "フィルタリング実行"))
                ]
                .OnClicked_Lambda([this]() {
                    // オプションにない地物タイプは全て含める
                    const auto HiddenFeatureTypes = ~(CityObject::CityObjectsType::COT_BuildingInstallation | CityObject::CityObjectsType::COT_Door |
                        CityObject::CityObjectsType::COT_Window | CityObject::CityObjectsType::COT_BuildingPart |
                        CityObject::CityObjectsType::COT_WallSurface | CityObject::CityObjectsType::COT_RoofSurface |
                        CityObject::CityObjectsType::COT_GroundSurface | CityObject::CityObjectsType::COT_ClosureSurface |
                        CityObject::CityObjectsType::COT_OuterCeilingSurface | CityObject::CityObjectsType::COT_OuterFloorSurface |
                        CityObject::CityObjectsType::COT_TINRelief | CityObject::CityObjectsType::COT_MassPointRelief |
                        CityObject::CityObjectsType::COT_SolitaryVegetationObject | CityObject::CityObjectsType::COT_PlantCover);
                    EnableCityObjects = EnableCityObjects | HiddenFeatureTypes;

                    SelectingActor->FilterByLODs(EnablePackage, TargetMinLOD, TargetMaxLOD, bShowMultiLOD)->FilterByFeatureTypes(EnableCityObjects);
                    return FReply::Handled();
                    })
                .ButtonColorAndOpacity(FSlateColor(FColor(0, 255, 255)))
                .Visibility_Lambda([this](){
                        bool IsVisible = SelectingActor != nullptr && !SelectingActor->IsFiltering();
                        return IsVisible ? EVisibility::Visible : EVisibility::Collapsed;
                    })
            ]

        + SHorizontalBox::Slot()
            .FillWidth(0.8f)
            [
                SNew(SButton)
                .Content()
            [SNew(STextBlock)
            .Justification(ETextJustify::Center)
            .Margin(FMargin(0, 5, 0, 5))
            .Text(LOCTEXT("Filtering", "フィルタリング実行中..."))
            ]
        .IsEnabled(false)
            .ButtonColorAndOpacity(FSlateColor(FColor(100, 100, 100)))
                .Visibility_Lambda([this]() {
                bool IsVisible = SelectingActor != nullptr && SelectingActor->IsFiltering();
                return IsVisible ? EVisibility::Visible : EVisibility::Collapsed;
                    })
            ]
            
             + SHorizontalBox::Slot()
            .FillWidth(0.1f)
        ]
    ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructHeader() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(STextBlock)
            .MinDesiredWidth(700)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHeader)
            .HAlign(HAlign_Center)
            .Content()
            [
                SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .TextStyle(Style, "PLATEAUEditor.Heading1")
                    .Text(LOCTEXT("FilteringHeader", "インポートしたモデルのフィルタリングを行います。"))
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .Padding(FMargin(0, 15, 0, 0))
        .AutoHeight()
        [
            SNew(SHeader)
            .Content()
            [
                SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .TextStyle(Style, "PLATEAUEditor.Heading2")
                    .Text(LOCTEXT("Filtering", "フィルタ条件指定"))
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(15,10,0,15))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(STextBlock)
                .Text(LOCTEXT("LODCheck","重複する地物を非表示"))
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.1f)

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SCheckBox)
                .IsChecked(true)
                .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                    bShowMultiLOD = NewState == ECheckBoxState::Checked;
                    })
                .IsChecked_Lambda([this]() {
                    return bShowMultiLOD ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                    })
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(15, 10, 0, 15))
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("AllCheck", "全選択"))
                .OnClicked_Lambda([this](){
                    SetAllFlags(true);
                    return FReply::Handled();
                    })
                .ButtonColorAndOpacity(FSlateColor(FColor(0, 255, 255)))
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()

            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("AllCheckRemove", "全選択を解除"))
                .OnClicked_Lambda([this]() {
                    SetAllFlags(false);
                    return FReply::Handled();
                    })
                .ButtonColorAndOpacity(FSlateColor(FColor(0, 255, 255)))
            ]
        ];
}

TSharedPtr<SBox> SPLATEAUFilteringPanel::ConstructBuilldingPanel() {
    return SNew(SBox)
        .Visibility_Lambda([this]() {
        if (((uint32_t)AvailablePackage & (uint32_t)PredefinedCityModelPackage::Building))
            return EVisibility::Visible;
        else
            return EVisibility::Collapsed;
            })
        [
             SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(15, 0, 0, 5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .IsChecked(true)
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                        SetEnableStatFromGenericPanel(plateau::dataset::PredefinedCityModelPackage::Building, NewState == ECheckBoxState::Checked);
                        if (NewState == ECheckBoxState::Unchecked) {
                            SetAllBuildingSettingFlag(false);
                        }
                        })
                    .IsChecked_Lambda([this](){
                            if (GetEnableStatFromGenericPanel(plateau::dataset::PredefinedCityModelPackage::Building))
                                return ECheckBoxState::Checked;
                            else
                                return ECheckBoxState::Unchecked;
                        })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("Building", " 建造物(Building)"))
                    .ColorAndOpacity_Lambda([this]() {
                        return GetEnableStatFromGenericPanel(plateau::dataset::PredefinedCityModelPackage::Building) ? PANEL_ENABLE_TEXT_COLOR : PANEL_DISABLE_TEXT_COLOR;
                        })
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1)
            ]
            + SVerticalBox::Slot()
            [
                ConstructBuildingDetailsPanel().ToSharedRef()
            ]
        ];
}

TSharedPtr<SBox> SPLATEAUFilteringPanel::ConstructReliefPanel() {
    return SNew(SBox)
        .Visibility_Lambda([this]() {
        if (((uint32_t)AvailablePackage & (uint32_t)PredefinedCityModelPackage::Relief))
            return EVisibility::Visible;
        else
            return EVisibility::Collapsed;
            })
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(15, 0, 0, 5)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SCheckBox)
                        .IsChecked(true)
                        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                            SetEnableStatFromGenericPanel(PredefinedCityModelPackage::Relief, NewState == ECheckBoxState::Checked);
                            if (NewState == ECheckBoxState::Unchecked) {
                                SetAllReliefSettingFlag(false);
                            }
                            })
                        .IsChecked_Lambda([this](){
                                if (GetEnableStatFromGenericPanel(PredefinedCityModelPackage::Relief))
                                    return ECheckBoxState::Checked;
                                else
                                    return ECheckBoxState::Unchecked;
                            })
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Relief", " 土地起伏(Relief)"))
                        .ColorAndOpacity_Lambda([this]() {
                            return GetEnableStatFromGenericPanel(PredefinedCityModelPackage::Relief) ? PANEL_ENABLE_TEXT_COLOR : PANEL_DISABLE_TEXT_COLOR;
                            })
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(1)
                ]

            + SVerticalBox::Slot()
            [
                ConstructReliefDetailsPanel().ToSharedRef()
            ]
        ];
}

TSharedPtr<SBox> SPLATEAUFilteringPanel::ConstructVegetationPanel() {
    return SNew(SBox)
        .Visibility_Lambda([this]() {
        if (((uint32_t)AvailablePackage & (uint32_t)PredefinedCityModelPackage::Vegetation))
            return EVisibility::Visible;
        else
            return EVisibility::Collapsed;
            })
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(15, 0, 0, 5)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SCheckBox)
                        .IsChecked(true)
                        .OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
                            SetEnableStatFromGenericPanel(PredefinedCityModelPackage::Vegetation, NewState == ECheckBoxState::Checked);
                            if (NewState == ECheckBoxState::Unchecked) {
                                SetAllVegetationSettingFlag(false);
                            }
                            })
                        .IsChecked_Lambda([this](){
                                if (GetEnableStatFromGenericPanel(PredefinedCityModelPackage::Vegetation))
                                    return ECheckBoxState::Checked;
                                else
                                    return ECheckBoxState::Unchecked;
                            })
                    ]

                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Vegetation", " 植生(Vegetation)"))
                        .ColorAndOpacity_Lambda([this]() {
                            return GetEnableStatFromGenericPanel(PredefinedCityModelPackage::Vegetation) ? PANEL_ENABLE_TEXT_COLOR : PANEL_DISABLE_TEXT_COLOR;
                            })
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(1)
            ] 
            + SVerticalBox::Slot()
            [
                ConstructVegetationDetailsPanel().ToSharedRef()
            ]
        ];
}

TSharedPtr<SBox> SPLATEAUFilteringPanel::ConstructGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage, const FText PanelTitle) {
    return SNew(SBox)
        .Visibility_Lambda([this,TargetPackage]() {
            return ((uint32_t)AvailablePackage & (uint32_t)TargetPackage) ? EVisibility::Visible : EVisibility::Collapsed;
            })
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(15, 0, 0, 5)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .IsChecked(true)
                    .OnCheckStateChanged_Lambda([this, TargetPackage](ECheckBoxState NewState) {
                        SetEnableStatFromGenericPanel(TargetPackage, NewState == ECheckBoxState::Checked);
                        })
                    .IsChecked_Lambda([this, TargetPackage](){
                            if (GetEnableStatFromGenericPanel(TargetPackage))
                                return ECheckBoxState::Checked;
                            else
                                return ECheckBoxState::Unchecked;
                        })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(PanelTitle)
                    .ColorAndOpacity_Lambda([this, TargetPackage]() {
                        return GetEnableStatFromGenericPanel(TargetPackage) ? PANEL_ENABLE_TEXT_COLOR : PANEL_DISABLE_TEXT_COLOR;
                        })
                ]
            ]
        
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .FillWidth(0.05f)
                [
                    SNew(SSpacer)
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1)
                [
                    SNew(SOverlay)
                    + SOverlay::Slot()
                    [
                        SNew(SBox)
                        .HeightOverride(50)
                        .Padding(FMargin(0, 0, 0, 0))
                        .RenderTransformPivot(FVector2D(0.0f, 0.0f))
                        [
                            SNew(SImage)
                            .Image(Style->GetBrush("PLATEAUEditor.TabBackground"))
                            .ColorAndOpacity(FSlateColor(FColor(0, 0, 0, 100)))
                        ]
                    ]

                    + SOverlay::Slot()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .Padding(20, 17, 0, 0)
                        [
                           ConstructGenericLODWidget(TargetPackage).ToSharedRef()
                        ]
                    ]
                ]
                + SHorizontalBox::Slot()
                .FillWidth(0.05f)
                [
                    SNew(SSpacer)
                ]
            ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructBuildingDetailsPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SBox)
                    .HeightOverride(330)
                    .Padding(FMargin(0, -30, 0, 0))
                    .RenderTransformPivot(FVector2D(0.0f, 0.0f))
                    [
                        SNew(SImage)
                        .Image(Style->GetBrush("PLATEAUEditor.TabBackground"))
                        .ColorAndOpacity(FSlateColor(FColor(0, 0, 0, 100)))
                    ]
                ]

                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .Padding(20, 17, 0, 10)
                    .AutoHeight()
                    [
                        ConstructBuildingLODWidget().ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_BuildingInstallation, LOCTEXT("InstallationCheck", "建物付属設備 (BuildingInstallation)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_Door, LOCTEXT("DoorCheck", "ドア (Door)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_Window, LOCTEXT("WindowCheck", "窓 (Window)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_BuildingPart, LOCTEXT("BuildingParCheck", "建築物パーツ (BuildingPart)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_WallSurface, LOCTEXT("WallSurfaceCheck", " 壁面 (WallSurface)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_RoofSurface, LOCTEXT("RoofSurfaceCheck", " 屋根面 (RoofSurface)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_GroundSurface, LOCTEXT("GroundSurfaceCheck", " 接地面 (GroundSurface)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_ClosureSurface, LOCTEXT("ClosureSurfaceCheck", " 開口部 (ClosureSurface)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_OuterCeilingSurface, LOCTEXT("OuterCellingSurfaceCheck", " 外側の天井 (OuterCellingSurface)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Building,
                            CityObject::CityObjectsType::COT_OuterFloorSurface, LOCTEXT("OuterFloorSurfaceCheck", " 屋根の通行可能部分 (OuterFloorSurface)")).ToSharedRef()
                    ]
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructReliefDetailsPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 0, 0, 0)
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SBox)
                    .HeightOverride(120)
                    .Padding(FMargin(0, -5, 0, 0))
                    .RenderTransformPivot(FVector2D(0.0f, 0.0f))
                    [
                        SNew(SImage)
                        .Image(Style->GetBrush("PLATEAUEditor.TabBackground"))
                        .ColorAndOpacity(FSlateColor(FColor(0, 0, 0, 100)))
                    ]
                ]

                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .Padding(20, 17, 0, 20)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        [
                           ConstructGenericLODWidget(PredefinedCityModelPackage::Relief).ToSharedRef()
                        ]
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Relief,
                            CityObject::CityObjectsType::COT_TINRelief, LOCTEXT("TINReliefCheck", " ポリゴンによる起伏表現 (TINRelief)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Relief,
                            CityObject::CityObjectsType::COT_MassPointRelief, LOCTEXT("MassPointRelief", " 点群による起伏表現 (MassPointRelief)")).ToSharedRef()
                    ]
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructVegetationDetailsPanel() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 0, 0, 0)
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]

            + SHorizontalBox::Slot()
            .FillWidth(1)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [
                    SNew(SBox)
                    .HeightOverride(120)
                    .Padding(FMargin(0, -5, 0, 0))
                    .RenderTransformPivot(FVector2D(0.0f, 0.0f))
                    [
                        SNew(SImage)
                        .Image(Style->GetBrush("PLATEAUEditor.TabBackground"))
                        .ColorAndOpacity(FSlateColor(FColor(0, 0, 0, 100)))
                    ]
                ]

                + SOverlay::Slot()
                [
                    SNew(SVerticalBox)

                    + SVerticalBox::Slot()
                    .Padding(20, 17, 0, 20)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        [
                            ConstructGenericLODWidget(PredefinedCityModelPackage::Vegetation).ToSharedRef()
                        ]
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Vegetation,
                            CityObject::CityObjectsType::COT_PlantCover, LOCTEXT("PlantCover", " 植生のまとまり (PlantCover)")).ToSharedRef()
                    ]

                    + SVerticalBox::Slot()
                    .Padding(20, 5, 0, 0)
                    .AutoHeight()
                    [
                        ConstructGenericCheckBox(PredefinedCityModelPackage::Vegetation,
                            CityObject::CityObjectsType::COT_SolitaryVegetationObject, LOCTEXT("SolitaryVegetationObject", " 単独の木 (SolitaryVegetationObject)")).ToSharedRef()
                    ]
                ]
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
            [
                SNew(SSpacer)
            ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructBuildingLODWidget() {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 10, 0, 0)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .FillWidth(0.0f)

            + SHorizontalBox::Slot()
            .FillWidth(0.06f)
            .Padding(0, 0, 5, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("BuildingMinLOD", " 最小LOD"))
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.1f)
            .Padding(0, -5, 10, 0)
            [
                SNew(SNumericEntryBox<int>)
                .AllowSpin(true)
                .MinSliderValue_Lambda([this]() {
                    if (SelectingActor != nullptr)
                        return TargetLODs[0].MinLOD;
                    else
                        return 0;
                    })
                .MaxSliderValue_Lambda([this]() {
                    if (SelectingActor != nullptr)
                        return TargetLODs[0].MaxLOD;
                    else
                        return 0;
                    })
                .OnValueChanged_Lambda(
                    [this](const int Value) {
                        if (Value <= TargetLODs[0].MaxLOD)
                            SetMinBuildingLOD(Value);
                    })
                .Value_Lambda(
                    [this]() {
                        return GetTargetMinLOD();
                    })
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.06f)
            .Padding(0, 0, 5, 0)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("BuildingMaxLOD", "最大LOD"))
            ]

            + SHorizontalBox::Slot()
            .FillWidth(0.1f)
            .Padding(0, -5, 0, 0)
            [
                SNew(SNumericEntryBox<int>)
                .AllowSpin(true)
                .MinSliderValue_Lambda([this]() {
                    if (SelectingActor != nullptr)
                        return TargetLODs[0].MinLOD;
                    else
                        return 0;
                    })
                .MaxSliderValue_Lambda([this]() {
                    if (SelectingActor != nullptr)
                        return TargetLODs[0].MaxLOD;
                    else
                        return 0;
                    })
                .OnValueChanged_Lambda(
                    [this](const int Value) {
                        if (Value >= TargetLODs[0].MinLOD) {
                            SetMaxBuildingLOD(Value);
                        }
                    })
                .Value_Raw(this, &SPLATEAUFilteringPanel::GetTargetMaxLOD)
            ]
            + SHorizontalBox::Slot()
            .FillWidth(0.05f)
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructGenericLODWidget(PredefinedCityModelPackage TargetPackage) {
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            [
                SNew(STextBlock)
                .Text_Lambda([this, TargetPackage] {
                    return GetLODText(TargetPackage);
                    })
            ]
        ];
}

FText SPLATEAUFilteringPanel::GetLODText(PredefinedCityModelPackage TargetPackage) {
    FString TmpString = "LOD ";
    if(SelectingActor != nullptr)
    {
        switch (TargetPackage) {
        case PredefinedCityModelPackage::Building:
            TmpString += std::to_string(TargetLODs[0].MaxLOD).c_str();
                break;
        case PredefinedCityModelPackage::Road:
            TmpString += std::to_string(TargetLODs[1].MaxLOD).c_str();
                break;
        case PredefinedCityModelPackage::UrbanPlanningDecision:
            TmpString += std::to_string(TargetLODs[2].MaxLOD).c_str();
                break;
        case PredefinedCityModelPackage::LandUse:
            TmpString += std::to_string(TargetLODs[3].MaxLOD).c_str();
            break;
        case PredefinedCityModelPackage::CityFurniture:
            TmpString += std::to_string(TargetLODs[4].MaxLOD).c_str();
            break;
        case PredefinedCityModelPackage::Vegetation:
            TmpString += std::to_string(TargetLODs[5].MaxLOD).c_str();
            break;
        case PredefinedCityModelPackage::Relief:
            TmpString += std::to_string(TargetLODs[6].MaxLOD).c_str();
            break;
        case PredefinedCityModelPackage::DisasterRisk:
            TmpString += std::to_string(TargetLODs[7].MaxLOD).c_str();
            break;
        case PredefinedCityModelPackage::Unknown:
            TmpString += std::to_string(TargetLODs[8].MaxLOD).c_str();
            break;
        default:
            TmpString += std::to_string(TargetLODs[0].MaxLOD).c_str();
            break;
        }
    }
    return FText::FromString(*TmpString);
}

TSharedPtr<SBox> SPLATEAUFilteringPanel::ConstructGenericCheckBox(const plateau::dataset::PredefinedCityModelPackage TargetPackage,
    const citygml::CityObject::CityObjectsType TargetObjectType, const FText CheckBoxTitle) {
    return SNew(SBox)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SCheckBox)
                    .IsChecked(true)
                    .OnCheckStateChanged_Lambda([this, TargetPackage, TargetObjectType](ECheckBoxState NewState) {
                        if(NewState == ECheckBoxState::Checked){
                            EnablePackage = EnablePackage | TargetPackage;
                            EnableCityObjects = EnableCityObjects | TargetObjectType;
                        }
                        else {
                            EnableCityObjects = EnableCityObjects & ~TargetObjectType;
                        }
                        })
                    .IsChecked_Lambda([this, TargetObjectType]() {
                            return ((uint64_t)EnableCityObjects & (uint64_t)TargetObjectType) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                        })
                ]

                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(0, 0, 0, 0)
                [
                    SNew(STextBlock)
                    .Text(CheckBoxTitle)
                    .ColorAndOpacity_Lambda([this, TargetObjectType]() {
                        return ((uint64_t)EnableCityObjects & (uint64_t)TargetObjectType) ? PANEL_ENABLE_TEXT_COLOR : PANEL_DISABLE_TEXT_COLOR;
                        })
                ]
            ]
        ];
}

TSharedPtr<SVerticalBox> SPLATEAUFilteringPanel::ConstructSelectingActorPanel() {
    return SNew(SVerticalBox)
        //選択オブジェクト表示ヘッダー
        + SVerticalBox::Slot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        [
            SNew(SHeader)
            .Content()
            [
                SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .TextStyle(Style, "PLATEAUEditor.Heading2")
                    .Text(LOCTEXT("Selecting Object", "選択オブジェクト"))
                ]
            ]
        ]

    //オブジェクト名表示部
    +SVerticalBox::Slot()
    .AutoHeight()
    .HAlign(HAlign_Fill)
    .Padding(FMargin(19, 15, 0, 15))
    [
        SNew(STextBlock)
        .Text(LOCTEXT("Object Name Here", "Object Name Here"))
        .Text_Lambda(
        [this]() {
            AActor* TargetActor = GEditor->GetSelectedActors()->GetTop<AActor>();
            if (TargetActor == nullptr) {
                AvailablePackage = PredefinedCityModelPackage::None;
                SelectingActor = nullptr;
                bActorSelected = false;
                return (LOCTEXT("Please select Actor from Outliner", "アウトライナーからアクターを選択してください"));
            }
            else if (Cast<APLATEAUInstancedCityModel>(TargetActor) == nullptr) {
                AvailablePackage = PredefinedCityModelPackage::None;
                SelectingActor = nullptr;
                bActorSelected = false;
                return (LOCTEXT("Please select PLATEAUInstancedCityModel Actor", "正しいアクター (PLATEAUInstancedCityModel) を選択してください"));
            }
            else {
                SelectingActor = Cast<APLATEAUInstancedCityModel>(TargetActor);
                CheckTargetLODs();
                bActorSelected = true;
                return FText::FromString(TargetActor->GetActorLabel());
            }
        })
    ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
