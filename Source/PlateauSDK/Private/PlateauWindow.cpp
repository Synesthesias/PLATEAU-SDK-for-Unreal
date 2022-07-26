
// Fill out your copyright notice in the Description page of Project Settings.


#include "PlateauWindow.h"

#include "LevelEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Templates/SharedPointer.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Dialogs/DlgPickPath.h"
#include <filesystem>
#include <vector>
#include <algorithm>

#include "citygml/citygml.h"
#include "plateau/mesh/mesh_converter.h"
#include "plateau/mesh/mesh_convert_options_factory.h"
#include "PLATEAUFileUtils.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPlateauSDKModule"

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

PlateauWindow::PlateauWindow() {
    m_existFeatures.Init(false, m_Features.Num());
}

void PlateauWindow::onMainFrameLoad(TSharedPtr<SWindow> inRootWindow, bool isNewProjectWindow) {
    if ((!isNewProjectWindow) && (inRootWindow.IsValid())) {
        m_rootWindow = inRootWindow;
    }
}

void PlateauWindow::onWindowMenuBarExtension(FMenuBarBuilder& menuBarBuilder) {
    menuBarBuilder.AddPullDownMenu(
        LOCTEXT("MenuBarTitle", "PLATEAU"),
        LOCTEXT("MenuBarToolkit", "PLATEAUメニューを開く."),
        FNewMenuDelegate::CreateRaw(this, &PlateauWindow::onPulldownMenuExtension)
    );
}

void PlateauWindow::onPulldownMenuExtension(FMenuBuilder& menuBuilder) {
    menuBuilder.AddMenuEntry(
        LOCTEXT("MenuTitle", "都市モデルをインポート"),
        LOCTEXT("PulldownMenuToolTip", "都市モデルインポート画面を開く."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &PlateauWindow::showPlateauWindow)));
}

void PlateauWindow::showPlateauWindow() {
    if (!m_myWindow.IsValid()) {
        TSharedPtr<SWindow> window = SNew(SWindow)
            .Title(LOCTEXT("Model File Converter Window", "都市モデルインポート画面"))
            .ClientSize(FVector2D(500.f, 800.f));
        m_myWindow = TWeakPtr<SWindow>(window);

        if (m_rootWindow.IsValid()) {
            FSlateApplication::Get().AddWindowAsNativeChild(
                window.ToSharedRef(), m_rootWindow.Pin().ToSharedRef());
        }
        updatePlateauWindow(m_myWindow);
    }
    m_myWindow.Pin()->BringToFront();
}

void PlateauWindow::updatePlateauWindow(TWeakPtr<SWindow> window) {
    auto scrollBox = SNew(SScrollBox);

#pragma region Select_gml_File
    scrollBox->AddSlot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .Text(LOCTEXT("Block1", "1. インポート元フォルダ選択"))
        .IsReadOnly(true)
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(20, 5, 20, 5))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &PlateauWindow::onBtnSelectGmlFileClicked)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Button1", "参照..."))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 10))
        [
            SNew(SEditableTextBox)
            .IsReadOnly(true)
        .Text(FText::FromString(m_gmlFolderPath))
        ]
        ];
#pragma  endregion

#pragma region select_region_mesh
    if (m_gmlFileSelected) {
        auto vbMeshCodes = SNew(SVerticalBox);
        int indexRegion = 0;
        bool selectSecondMesh;
        bool selectThirdMesh;
        TArray<MeshCode> thirdMesh;
        std::vector<MeshCode> tempMeshCodes;

        m_secondMesh.Reset();
        for (auto meshCode : *m_meshCodes) {
            int meshLevel = meshCode.getLevel();
            if (FString(meshCode.get().c_str()).Len() == 6) {
                m_secondMesh.Add(meshCode);
            } else {
                thirdMesh.Add(meshCode);
            }
        }

        //疑似2次メッシュ作成
        for (auto mesh3 : thirdMesh) {
            auto sim = MeshCode(mesh3.getAsLevel2());
            if (m_secondMesh.Find(sim) == INDEX_NONE) {
                m_secondMesh.Add(sim);
                m_selectRegion.Add(false);
            }
        }

        m_secondMesh = sortMeshCodes(m_secondMesh);
        thirdMesh = sortMeshCodes(thirdMesh);

        vbMeshCodes->AddSlot()
            .Padding(FMargin(0, 0, 0, 3))[
                SNew(STextBlock)
                    .Text(FText::FromString((FString(TEXT("含める地域")))))
            ];

        vbMeshCodes->AddSlot()
            .Padding(FMargin(0, 0, 0, 3))
            .AutoHeight()[
                SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(FMargin(0, 0, 15, 0))
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全選択"))))
                    .OnClicked_Raw(this, &PlateauWindow::onBtnAllSecondMeshSelectClicked)
                    ]
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全除外"))))
                    .OnClicked_Raw(this, &PlateauWindow::onBtnAllSecondMeshRelieveClicked)
                    ]
            ];

        for (auto sMesh : m_secondMesh) {
            selectSecondMesh = m_selectRegion[indexRegion];
            tempMeshCodes.push_back(sMesh);
            vbMeshCodes->AddSlot()
                .AutoHeight()
                .Padding(FMargin(0, 0, 0, 3))[
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString(FString(sMesh.get().c_str())))
                        ]
                    + SHorizontalBox::Slot()
                        [
                            SNew(SCheckBox)
                            .IsChecked(selectSecondMesh)
                        .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectRegion, indexRegion, sMesh.get())
                        ]
                ];
            indexRegion++;

            thirdMesh = sortMeshCodes(thirdMesh);
            bool btnDisplayed = false;
            for (auto tMesh : thirdMesh) {
                if (FString(tMesh.get().c_str()).Contains(FString(sMesh.get().c_str()))) {
                    tempMeshCodes.push_back(tMesh);
                    if (selectSecondMesh) {
                        selectThirdMesh = m_selectRegion[indexRegion];

                        if (!btnDisplayed) {
                            vbMeshCodes->AddSlot()
                                .Padding(FMargin(0, 0, 0, 3))
                                .AutoHeight()[
                                    SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(FMargin(30, 0, 15, 0))
                                        [
                                            SNew(SButton)
                                            .Text(FText::FromString(FString(TEXT("全選択"))))
                                        .OnClicked_Raw(this, &PlateauWindow::onBtnThirdMeshSelectClicked, sMesh.get())
                                        ]
                                    + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        [
                                            SNew(SButton)
                                            .Text(FText::FromString(FString(TEXT("全除外"))))
                                        .OnClicked_Raw(this, &PlateauWindow::onBtnThirdMeshRelieveClicked, sMesh.get())
                                        ]
                                ];

                            btnDisplayed = true;
                        }

                        vbMeshCodes->AddSlot()
                            .AutoHeight()
                            .Padding(FMargin(30, 0, 0, 3))[
                                SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    [
                                        SNew(STextBlock)
                                        .Text(FText::FromString(FString(tMesh.get().c_str())))
                                    ]
                                + SHorizontalBox::Slot()
                                    [
                                        SNew(SCheckBox)
                                        .IsChecked(selectThirdMesh)
                                    .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectRegion, indexRegion, tMesh.get())
                                    ]
                            ];
                    }
                    indexRegion++;
                }
            }
        }
        m_meshCodes = std::make_shared<std::vector<MeshCode>>(tempMeshCodes);
        if ((*m_meshCodes).size() != 0) {
            scrollBox->AddSlot()[
                vbMeshCodes
            ];
        }

#pragma endregion

#pragma region select_feature_mesh
        if (m_selectFeatureSize != 0) {
            auto vbFeatureMesh = SNew(SVerticalBox);
            int selectIndex = 0;

            vbFeatureMesh->AddSlot()
                .Padding(FMargin(0, 20, 0, 0))[
                    SNew(STextBlock)
                        .Text(FText::FromString(FString(TEXT("含める地物"))))
                ];

            vbFeatureMesh->AddSlot()
                .Padding(FMargin(0, 0, 0, 8))
                .AutoHeight()[
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(0, 0, 10, 0))
                        [
                            SNew(SButton)
                            .Text(FText::FromString(FString(TEXT("全選択"))))
                        .OnClicked_Raw(this, &PlateauWindow::onBtnAllFeatureSelectClicked)
                        ]
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SButton)
                            .Text(FText::FromString(FString(TEXT("全除外"))))
                        .OnClicked_Raw(this, &PlateauWindow::onBtnAllFeatureRelieveClicked)
                        ]
                ];

            for (int j = 0; j < m_existFeatures.Num(); j++) {
                if (m_existFeatures[j]) {
                    bool checkFeature = m_selectFeature[selectIndex];
                    vbFeatureMesh->AddSlot()
                        .Padding(FMargin(0, 0, 0, 3))[
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                [
                                    SNew(STextBlock)
                                    .Text(FCityModelPlacementSettings::GetDisplayName(m_Features[j]))
                                ]
                            + SHorizontalBox::Slot()
                                [
                                    SNew(SCheckBox)
                                    .IsChecked(checkFeature)
                                .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectFeature, selectIndex)
                                ]
                        ];
                    selectIndex++;
                }
            }

            scrollBox->AddSlot()[
                vbFeatureMesh
            ];
        }
    }

#pragma endregion

#pragma region Configure
    scrollBox->AddSlot()
        [
            SNew(SVerticalBox)
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
                        m_includeAppearance = State != ECheckBoxState::Unchecked;
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
                        MeshGranularity Granularity = ItemIter->Key;
                        FUIAction ItemAction(FExecuteAction::CreateLambda(
                            [this, Granularity]() {
                                m_meshGranularity = Granularity;
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
                            return GetMeshGranularityTexts()[m_meshGranularity];
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
        ];
#pragma endregion

#pragma region Convert
    scrollBox->AddSlot()
        [
            SNew(SVerticalBox)
    + SVerticalBox::Slot()
        .Padding(FMargin(20, 5, 20, 20))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &PlateauWindow::onBtnConvertClicked)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Button4", "出力"))
        ]
        ]

        ];

#pragma endregion

    window.Pin()->SetContent(scrollBox);
}

FReply PlateauWindow::onBtnSelectGmlFileClicked() {
    void* windowHandle = m_myWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    FString dialogTitle = FString("Select folder.");
    FString defaultPath = m_gmlFolderPath;
    FString outFolderName;

    IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();

    m_gmlFileSelected = desktopPlatform->OpenDirectoryDialog(
        windowHandle,
        dialogTitle,
        defaultPath,
        outFolderName);

    if (m_gmlFileSelected) {
        m_gmlFolderPath = outFolderName;
        std::string rootPath = TCHAR_TO_UTF8(*(m_gmlFolderPath + "/udx"));
        m_collection = UdxFileCollection::find(rootPath);
        m_meshCodes = m_collection.getMeshCodes();

        m_selectRegion.Reset();
        m_selectRegion.Init(false, (*m_meshCodes).size());

        updatePlateauWindow(m_myWindow.Pin());
    }

    return FReply::Handled();
}

FReply PlateauWindow::onBtnSelectObjDestinationClicked() {
    TSharedRef<SDlgPickPath> PickContentPathDlg =
        SNew(SDlgPickPath)
        .Title(LOCTEXT("ChooseSaveObjFilePath", "Choose Location for saving obj file"));

    if (PickContentPathDlg->ShowModal() == EAppReturnType::Cancel) {
        return FReply::Handled();
    }

    FString path = PickContentPathDlg->GetPath().ToString();
    FString leftS, rightS;
    path.Split(TEXT("/Game/"), &leftS, &rightS);
    m_objFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + rightS);
    updatePlateauWindow(m_myWindow.Pin());

    return FReply::Handled();
}

FReply PlateauWindow::onBtnConvertClicked() {
    try {
        int selectIndex = 0;
        TArray<std::string>checkSubFolder = {
            UdxSubFolder::bldg().name(),
            UdxSubFolder::tran().name(),
            UdxSubFolder::frn().name(),
            UdxSubFolder::dem().name(),
            UdxSubFolder::veg().name()
        };
        TArray<FString> CopiedGmlFiles;
        for (int i = 0; i < m_existFeatures.Num(); i++) {
            if (!m_existFeatures[i])continue;
            if (!m_selectFeature[selectIndex]) {
                selectIndex++;
                continue;
            }
            for (auto subfolder : *m_subFolders) {
                if (i < m_existFeatures.Num() - 1 && subfolder.name() == checkSubFolder[i]) {
                    const auto GmlFileVector = m_filteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), subfolder);
                    for (const auto& GmlFile : *GmlFileVector) {
                        CopiedGmlFiles.Add(UTF8_TO_TCHAR(GmlFile.c_str()));
                    }
                } else if (i == m_existFeatures.Num() - 1 && checkSubFolder.Find(subfolder.name()) == INDEX_NONE) {
                    const auto GmlFileVector = m_filteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), subfolder);
                    for (const auto& GmlFile : *GmlFileVector) {
                        CopiedGmlFiles.Add(UTF8_TO_TCHAR(GmlFile.c_str()));
                    }
                }
            }
            selectIndex++;
        }

        if (CopiedGmlFiles.Num() == 0)
            return FReply::Handled();

        m_filteredCollection.copyCodelistFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")));

        // メッシュ変換設定
        ParserParams ParserParams;
        ParserParams.tesselate = false;
        const auto FirstCityModel = citygml::load(TCHAR_TO_UTF8(*CopiedGmlFiles[0]), ParserParams);
        for (auto& Entry : MeshConvertOptionsMap) {
            auto& Options = Entry.Value;
            Options.unit_scale = 0.01;
            Options.mesh_axes = AxesConversion::NWU;
            Options.export_appearance = m_includeAppearance;
            Options.mesh_granularity = m_meshGranularity;
            MeshConvertOptionsFactory::setValidReferencePoint(Options, *FirstCityModel);
        }

        TSharedRef<SDlgPickPath> PickContentPathDlg =
            SNew(SDlgPickPath)
            .Title(LOCTEXT("ChooseImportRootContentPath", "メッシュの出力先選択"));

        if (PickContentPathDlg->ShowModal() == EAppReturnType::Cancel) {
            return FReply::Handled();
        }

        FString path = PickContentPathDlg->GetPath().ToString();
        PLATEAUFileUtils::ImportFbx(CopiedGmlFiles, path, MeshConvertOptionsMap);

        return FReply::Handled();
    }
    catch (std::exception& e) {
        const auto errorLog = ANSI_TO_TCHAR(e.what());
        UE_LOG(LogTemp, Warning, TEXT("%s"), errorLog);
        return FReply::Handled();
    }
}

FReply PlateauWindow::onBtnAllSecondMeshSelectClicked() {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        if (m_secondMesh.Find((*m_meshCodes)[i]) != INDEX_NONE) {
            m_selectRegion[i] = true;
        }
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

FReply PlateauWindow::onBtnAllSecondMeshRelieveClicked() {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        m_selectRegion[i] = false;
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

FReply PlateauWindow::onBtnThirdMeshSelectClicked(std::string secondMesh) {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        if ((*m_meshCodes)[i].getAsLevel2() == secondMesh && (*m_meshCodes)[i].get() != secondMesh) {
            m_selectRegion[i] = true;
        }
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

FReply PlateauWindow::onBtnThirdMeshRelieveClicked(std::string secondMesh) {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        if ((*m_meshCodes)[i].getAsLevel2() == secondMesh && (*m_meshCodes)[i].get() != secondMesh) {
            m_selectRegion[i] = false;
        }
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

void PlateauWindow::onToggleCbSelectRegion(ECheckBoxState checkState, int num, std::string meshName) {
    m_selectRegion[num] = (checkState == ECheckBoxState::Checked);
    if (FString(meshName.c_str()).Len() == 6 && checkState == ECheckBoxState::Unchecked) {
        for (int i = 0; i < m_selectRegion.Num(); i++) {
            if ((*m_meshCodes)[i].getAsLevel2() == meshName) {
                m_selectRegion[i] = false;
            }
        }
    }

    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
}

FReply PlateauWindow::onBtnAllFeatureSelectClicked() {
    for (int i = 0; i < m_selectFeature.Num(); i++) {
        m_selectFeature[i] = true;
    }
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

FReply PlateauWindow::onBtnAllFeatureRelieveClicked() {
    for (int i = 0; i < m_selectFeature.Num(); i++) {
        m_selectFeature[i] = false;
    }
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

void PlateauWindow::onToggleCbSelectFeature(ECheckBoxState checkState, int index) {
    m_selectFeature[index] = (checkState == ECheckBoxState::Checked);
    updatePlateauWindow(m_myWindow.Pin());
}

void PlateauWindow::startup() {
    FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
    m_extender = MakeShared<FExtender>();
    if (m_extender.IsValid()) {
        m_extender->AddMenuBarExtension(
            "Help",
            EExtensionHook::After,
            nullptr,
            FMenuBarExtensionDelegate::CreateRaw(this, &PlateauWindow::onWindowMenuBarExtension)
        );
    }
    const auto menuExtensibilityManager = levelEditorModule.GetMenuExtensibilityManager();
    if (menuExtensibilityManager.IsValid()) {
        menuExtensibilityManager->AddExtender(m_extender);
    }

    IMainFrameModule& mainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
    mainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &PlateauWindow::onMainFrameLoad);
}

void PlateauWindow::shutdown() {
    if (m_extender.IsValid() && FModuleManager::Get().IsModuleLoaded(LEVEL_EDITOR_NAME)) {
        FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
        const auto menuExtensibilityManager = levelEditorModule.GetMenuExtensibilityManager();
        if (menuExtensibilityManager.IsValid()) {
            menuExtensibilityManager->RemoveExtender(m_extender);
        }
    }

    if (FModuleManager::Get().IsModuleLoaded("MainFrame")) {
        IMainFrameModule& mainFrameModule =
            FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
        mainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
    }
}

void PlateauWindow::checkRegionMesh() {
    std::vector<MeshCode> targetMeshCodes;

    int i = 0;
    for (auto meshCode : *m_meshCodes) {
        if (m_selectRegion[i]) {
            targetMeshCodes.push_back(meshCode);
        }
        i++;
    }
    m_existFeatures.Init(false, m_Features.Num());
    m_selectFeatureSize = 0;
    if (targetMeshCodes.size() != 0) {

        m_filteredCollection = UdxFileCollection::filter(m_collection, targetMeshCodes);
        m_subFolders = m_filteredCollection.getSubFolders();

        for (auto subfolder : *m_subFolders) {
            if (subfolder.name() == "bldg") {
                m_existFeatures[0] = true;
            } else if (subfolder.name() == "tran") {
                m_existFeatures[1] = true;
            } else if (subfolder.name() == "frn") 
            {
                m_existFeatures[2] = true;
            } else if (subfolder.name() == "dem") {
                m_existFeatures[3] = true;
            } else if (subfolder.name() == "veg") {
                m_existFeatures[4] = true;
            } else {
                m_existFeatures[5] = true;
            }
        }
        for (bool flg : m_existFeatures) {
            if (flg) m_selectFeatureSize++;
        }
        m_selectFeature.Init(false, m_selectFeatureSize);
    }
}

TArray<MeshCode> PlateauWindow::sortMeshCodes(TArray<MeshCode> meshArray) {
    std::vector<int> intVecArray;
    TArray<MeshCode> returnArray;
    for (auto mesh : meshArray) {
        intVecArray.push_back(atoi(mesh.get().c_str()));
    }

    std::sort(intVecArray.begin(), intVecArray.end());
    for (int meshNum : intVecArray) {
        for (auto mesh_ : meshArray) {
            if (std::to_string(meshNum) == mesh_.get()) {
                returnArray.Add(mesh_);
            }
        }
    }

    return returnArray;
}


TSharedRef<SVerticalBox> PlateauWindow::CreateLODSettingsPanel(ECityModelPackage Package) {
    MeshConvertOptionsMap.FindOrAdd(Package, MeshConvertOptions()).min_lod = Package == ECityModelPackage::Building || Package == ECityModelPackage::Others ? 0 : 1;
    return SNew(SVerticalBox).Visibility_Lambda(
        [this, Package]() {
            int selectIndex = 0;
            for (int i = 0; i < m_Features.Num(); ++i) {
                if (m_existFeatures[i]) {
                    if (m_selectFeature[selectIndex] && Package == m_Features[i]) {
                        return EVisibility::Visible;
                    }
                    selectIndex++;
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
