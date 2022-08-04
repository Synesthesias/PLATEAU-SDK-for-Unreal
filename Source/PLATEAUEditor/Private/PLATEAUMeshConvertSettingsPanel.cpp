// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUMeshConvertSettingsPanel.h"
#include "citygml/citygml.h"
#include "plateau/mesh/mesh_converter.h"
#include "plateau/mesh/mesh_convert_options_factory.h"
#include "PLATEAUFileUtils.h"
#include "Widgets/Input/SSlider.h"
#include "Dialogs/DlgPickPath.h"

#define LOCTEXT_NAMESPACE "FPLATEUEditorModule"


namespace {
    TMap<MeshGranularity, FText> GetMeshGranularityTexts() {
        TMap<MeshGranularity, FText> Items;
        Items.Add(MeshGranularity::PerPrimaryFeatureObject, LOCTEXT("PerPrimaryFeatureObject", "主要地物単位"));
        Items.Add(MeshGranularity::PerAtomicFeatureObject, LOCTEXT("PerAtomicFeatureObject", "最小地物単位"));
        Items.Add(MeshGranularity::PerCityModelArea, LOCTEXT("PerCityModelArea", "都市モデル地域単位"));
        return Items;
    }

    TMap<bool, FText> GetLODModeTexts() {
        TMap<bool, FText> Items;
        Items.Add(true, LOCTEXT("AllLODs", "全てのLOD"));
        Items.Add(false, LOCTEXT("OnlyMaxLOD", "最大LODのみ"));
        return Items;
    }
}

FPLATEAUMeshConvertSettingsPanel::FPLATEAUMeshConvertSettingsPanel() {
    ExistFeatures.Init(false, Features.Num());
    SelectFeatures.Init(false, Features.Num());
}

TSharedRef<SVerticalBox> FPLATEAUMeshConvertSettingsPanel::CreateConvertSettingsPanel() {
    return  SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .Text(LOCTEXT("MeshSettings", "メッシュ設定"))
        .IsReadOnly(true)
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]

    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 0))[
            SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("IncludeAppearance", "テクスチャを含める"))
                ]
            + SHorizontalBox::Slot()
                [
                    SNew(SCheckBox)
                    .IsChecked(true)
                .OnCheckStateChanged_Lambda(
                    [this](ECheckBoxState State) {
                        IncludeAppearance = State != ECheckBoxState::Unchecked;
                    })
                ]
        ]

        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 10)
            [SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            [SNew(STextBlock)
            .Text(FText::FromString(TEXT("メッシュ結合単位")))]
        + SHorizontalBox::Slot()
            [SNew(SComboButton)
            .OnGetMenuContent_Lambda(
                [this]() {
                    FMenuBuilder MenuBuilder(true, nullptr);
                    const auto Items = GetMeshGranularityTexts();
                    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                        FText ItemText = ItemIter->Value;
                        MeshGranularity Gran = ItemIter->Key;
                        FUIAction ItemAction(FExecuteAction::CreateLambda(
                            [this, Gran]() {
                                Granularity = Gran;
                            }));
                        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                    }
                    return MenuBuilder.MakeWidget();
                })
            .ContentPadding(0.0f)
                    //.ButtonStyle(FEditorStyle::Get(), "ToggleButton")
                    //.ForegroundColor(FSlateColor::UseForeground())
                    .VAlign(VAlign_Center)
                    .ButtonContent()
                    [SNew(STextBlock).Text_Lambda(
                        [this]() {
                            return GetMeshGranularityTexts()[Granularity];
                        })]]]

        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::Building)]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::Road)]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::Relief)]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::UrbanFacility)]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::Vegetation)]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [CreateLODSettingsPanel(ECityModelPackage::Others)]

        + SVerticalBox::Slot()
            .Padding(FMargin(20, 5, 20, 20))
            [
                SNew(SButton)
                .VAlign(VAlign_Center)
            .ForegroundColor(FColor::White)
            .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
            .OnClicked_Raw(this, &FPLATEAUMeshConvertSettingsPanel::OnBtnConvertClicked)
            .Content()
            [
                SNew(STextBlock)
                .Justification(ETextJustify::Center)
            .Margin(FMargin(0, 5, 0, 5))
            .Text(LOCTEXT("Button4", "出力"))
            ]
            ];
}

TSharedRef<SVerticalBox> FPLATEAUMeshConvertSettingsPanel::CreateLODSettingsPanel(ECityModelPackage Package) {
    MeshConvertOptionsMap.FindOrAdd(Package, MeshConvertOptions()).min_lod = Package == ECityModelPackage::Building || Package == ECityModelPackage::Others ? 0 : 1;
    return SNew(SVerticalBox).Visibility_Lambda(
        [this, Package]() {
            for (int i = 0; i < Features.Num(); ++i) {
                if (ExistFeatures[i]) {
                    if (SelectFeatures[i] && Package == Features[i]) {
                        return EVisibility::Visible;
                    }
                }
            }
            return EVisibility::Collapsed;
        })
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 10, 0, 0)
            [SNew(STextBlock)
            .Text(FCityModelPlacementSettings::GetDisplayName(Package))
            ]
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(20, 0, 0, 0)
            [SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("出力モード")))
            ]
        + SHorizontalBox::Slot()
            [SNew(SComboButton)
            .OnGetMenuContent_Lambda(
                [this, Package]() {
                    FMenuBuilder MenuBuilder(true, nullptr);
                    const auto Items = GetLODModeTexts();
                    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                        FText ItemText = ItemIter->Value;
                        bool Mode = ItemIter->Key;
                        FUIAction ItemAction(FExecuteAction::CreateLambda(
                            [this, Mode, Package]() {
                                MeshConvertOptionsMap[Package].export_lower_lod = Mode;
                            }));
                        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                    }
                    return MenuBuilder.MakeWidget();
                })
            .ContentPadding(0.0f)
                    //.ButtonStyle(FEditorStyle::Get(), "ToggleButton")
                    //.ForegroundColor(FSlateColor::UseForeground())
                    .VAlign(VAlign_Center)
                    .ButtonContent()
                    [SNew(STextBlock).Text_Lambda(
                        [this, Package]() {
                            return GetLODModeTexts()[MeshConvertOptionsMap[Package].export_lower_lod];
                        })]
            ]]

        // 最小LODスライダー
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [SNew(SSlider)
            .MaxValue(3)
            .MinValue(Package == ECityModelPackage::Building || Package == ECityModelPackage::Others ? 0 : 1)
            .StepSize(1)
            .MouseUsesStep(true)
            .Value(MeshConvertOptionsMap[Package].min_lod)
            .OnValueChanged_Lambda(
                [this, Package](int Value) {
                    MeshConvertOptionsMap[Package].min_lod = Value;
                })]

        // 最大LODスライダー
        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 0)
            [SNew(SSlider)
            .MaxValue(3)
            .MinValue(Package == ECityModelPackage::Building || Package == ECityModelPackage::Others ? 0 : 1)
            .StepSize(1)
            .MouseUsesStep(true)
            .Value(MeshConvertOptionsMap[Package].max_lod)
            .OnValueChanged_Lambda(
                [this, Package](int Value) {
                    MeshConvertOptionsMap[Package].max_lod = Value;
                })]

        + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 5, 0, 0)
            [
                SNew(STextBlock)
                .Text_Lambda(
                    [this, Package]() {
                        return  FText::Format(
                            LOCTEXT("LODRangeFormat", "最小LOD: {0}, 最大LOD: {1}"),
                            FText::AsNumber(MeshConvertOptionsMap[Package].min_lod),
                            FText::AsNumber(MeshConvertOptionsMap[Package].max_lod));
                    })
            ]
            ];
}

void FPLATEAUMeshConvertSettingsPanel::UpdateFeaturesInfo(TArray<bool> ExistArray, TArray<bool> SelectArray, UdxFileCollection Collection) {
    ExistFeatures = ExistArray;
    SelectFeatures = SelectArray;
    FilteredCollection = Collection;
}

FReply FPLATEAUMeshConvertSettingsPanel::OnBtnConvertClicked() {
    try {
        TArray<std::string>CheckSubFolder = {
            UdxSubFolder::bldg().name(),
            UdxSubFolder::tran().name(),
            UdxSubFolder::frn().name(),
            UdxSubFolder::dem().name(),
            UdxSubFolder::veg().name()
        };
        TArray<FString> CopiedGmlFiles;
        auto SubFolders = FilteredCollection.getSubFolders();
        for (int i = 0; i < ExistFeatures.Num(); i++) {
            if (!ExistFeatures[i] || !SelectFeatures[i])continue;
            for (auto Subfolder : *SubFolders) {
                if (i < ExistFeatures.Num() - 1 && Subfolder.name() == CheckSubFolder[i]) {
                    const auto GmlFileVector = FilteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), Subfolder);
                    for (const auto& GmlFile : *GmlFileVector) {
                        CopiedGmlFiles.Add(UTF8_TO_TCHAR(GmlFile.c_str()));
                    }
                }
                else if (i == ExistFeatures.Num() - 1 && CheckSubFolder.Find(Subfolder.name()) == INDEX_NONE) {
                    const auto GmlFileVector = FilteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), Subfolder);
                    for (const auto& GmlFile : *GmlFileVector) {
                        CopiedGmlFiles.Add(UTF8_TO_TCHAR(GmlFile.c_str()));
                    }
                }
            }
        }

        if (CopiedGmlFiles.Num() == 0)
            return FReply::Handled();

        FilteredCollection.copyCodelistFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")));

        // メッシュ変換設定
        ParserParams ParserParams;
        ParserParams.tesselate = false;
        const auto FirstCityModel = citygml::load(TCHAR_TO_UTF8(*CopiedGmlFiles[0]), ParserParams);
        for (auto& Entry : MeshConvertOptionsMap) {
            auto& Options = Entry.Value;
            Options.unit_scale = 0.01;
            Options.mesh_axes = AxesConversion::NWU;
            Options.export_appearance = IncludeAppearance;
            Options.mesh_granularity = Granularity;
            MeshConvertOptionsFactory::setValidReferencePoint(Options, *FirstCityModel);
        }

        TSharedRef<SDlgPickPath> PickContentPathDlg =
            SNew(SDlgPickPath)
            .Title(LOCTEXT("ChooseImportRootContentPath", "メッシュの出力先選択"));

        if (PickContentPathDlg->ShowModal() == EAppReturnType::Cancel) {
            return FReply::Handled();
        }

        FString Path = PickContentPathDlg->GetPath().ToString();
        PLATEAUFileUtils::ImportFbx(CopiedGmlFiles, Path, MeshConvertOptionsMap);

        return FReply::Handled();
    }
    catch (std::exception& e) {
        const auto ErrorLog = ANSI_TO_TCHAR(e.what());
        UE_LOG(LogTemp, Warning, TEXT("%s"), ErrorLog);
        return FReply::Handled();
    }

}
