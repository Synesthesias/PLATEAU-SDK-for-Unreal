
// Fill out your copyright notice in the Description page of Project Settings.


#include "PlateauWindow.h"

#include "LevelEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSpinBox.h"
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
#include "PLATEAUFileUtils.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPlateauSDKModule"


PlateauWindow::PlateauWindow() {
    m_outputModeArray.Add(MakeShareable(new FString(TEXT("全てのLOD"))));
    m_outputModeArray.Add(MakeShareable(new FString(TEXT("最大LODのみ"))));

    m_existFeatures.Init(false, m_Features.Num());
}

void PlateauWindow::onMainFrameLoad(TSharedPtr<SWindow> inRootWindow, bool isNewProjectWindow) {
    if ((!isNewProjectWindow) && (inRootWindow.IsValid())) {
        m_rootWindow = inRootWindow;
    }
}

void PlateauWindow::onWindowMenuBarExtension(FMenuBarBuilder& menuBarBuilder) {
    menuBarBuilder.AddPullDownMenu(
        LOCTEXT("MenuBarTitle", "Plateau"),
        LOCTEXT("MenuBarToolkit", "Open Plateau menu."),
        FNewMenuDelegate::CreateRaw(this, &PlateauWindow::onPulldownMenuExtension)
    );
}

void PlateauWindow::onPulldownMenuExtension(FMenuBuilder& menuBuilder) {
    menuBuilder.AddMenuEntry(
        LOCTEXT("MenuTitle", "Model File Converter Window"),
        LOCTEXT("PulldownMenuToolTip", "Show Model File Converter Window."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &PlateauWindow::showPlateauWindow)));
}

void PlateauWindow::showPlateauWindow() {
    if (!m_myWindow.IsValid()) {
        TSharedPtr<SWindow> window = SNew(SWindow)
            .Title(LOCTEXT("Model File Converter Window", "Model File Converter"))
            .ClientSize(FVector2D(500.f, 500.f));
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
        .Text(LOCTEXT("Block1", "1.Select gml File"))
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
        .Text(LOCTEXT("Button1", "Select gml File"))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 2, 0, 5))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("TEXT1", "gml file path:"))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 30))
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
        TArray<MeshCode> secondMesh;
        TArray<MeshCode> thirdMesh;
        TArray<MeshCode> delThirdMesh;
        std::vector<MeshCode> tempMeshCodes;

        for (auto meshCode : *m_meshCodes) {
            if (FString(meshCode.get().c_str()).Len() == 6) {
                secondMesh.Add(meshCode);
            }
            else {
                thirdMesh.Add(meshCode);
            }
        }

        for (auto sMesh : secondMesh) {
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
                        .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectRegion, indexRegion)
                        ]
                ];
            indexRegion++;

            thirdMesh = sortMeshCodes(thirdMesh);
            for (auto tMesh : thirdMesh) {
                if (FString(tMesh.get().c_str()).Contains(FString(sMesh.get().c_str()))) {
                    delThirdMesh.Add(tMesh);
                    tempMeshCodes.push_back(tMesh);
                    if (selectSecondMesh) {
                        selectThirdMesh = m_selectRegion[indexRegion];

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
                                    .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectRegion, indexRegion)
                                    ]
                            ];
                    }
                    indexRegion++;
                }
            }
            for (auto delTMesh : delThirdMesh) {
                thirdMesh.Remove(delTMesh);
            }
        }
        //2次メッシュのない３次メッシュ
        for (auto tMesh : thirdMesh) {
            delThirdMesh.Add(tMesh);
            tempMeshCodes.push_back(tMesh);
            selectThirdMesh = m_selectRegion[indexRegion];

            vbMeshCodes->AddSlot()
                .AutoHeight()
                .Padding(FMargin(0, 0, 0, 3))[
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
                        .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbSelectRegion, indexRegion)
                        ]
                ];
            indexRegion++;
        }

        m_meshCodes = std::make_shared<std::vector<MeshCode>>(tempMeshCodes);

        vbMeshCodes->AddSlot()
            .Padding(FMargin(0, 0, 0, 3))[
                SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(FMargin(0, 0, 15, 0))
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全選択"))))
                    .OnClicked_Raw(this, &PlateauWindow::onBtnAllRegionSelectClicked)
                    ]
                + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(SButton)
                        .Text(FText::FromString(FString(TEXT("全除外"))))
                    .OnClicked_Raw(this, &PlateauWindow::onBtnAllRegionRelieveClicked)
                    ]
            ];

        if ((*m_meshCodes).size() != 0) {
            scrollBox->AddSlot()[
                vbMeshCodes
            ];
        }

#pragma endregion

#pragma region select_feature_mesh
        if (m_selectFeature.Num() != 0) {

            auto vbRegionMesh = SNew(SVerticalBox);
            int selectIndex = 0;
            for (int j = 0; j < m_existFeatures.Num(); j++) {
                if (m_existFeatures[j]) {
                    bool checkFeature = m_selectFeature[selectIndex];
                    vbRegionMesh->AddSlot()
                        .Padding(FMargin(0, 0, 0, 3))[
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                [
                                    SNew(STextBlock)
                                    .Text(FText::FromString(m_Features[j]))
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

            vbRegionMesh->AddSlot()
                .Padding(FMargin(0, 0, 0, 8))[
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(0, 0, 15, 0))
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
            scrollBox->AddSlot()[
                vbRegionMesh
            ];
        }
    }

#pragma endregion

#pragma region Select_obj_File_Destination
    scrollBox->AddSlot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .Text(LOCTEXT("Block2", "2.Select obj File Destination"))
        .IsReadOnly(true)
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
    + SVerticalBox::Slot()
        .Padding(FMargin(20, 5, 20, 5))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &PlateauWindow::onBtnSelectObjDestinationClicked)
        .Content()
        [
            SNew(STextBlock)
            .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Button2", "Select obj Destination"))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 2, 0, 5))
        [
            SNew(STextBlock)
            .Text(LOCTEXT("TEXT2", "Destination obj file path:"))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 30))
        [
            SNew(SEditableTextBox)
            .IsReadOnly(true)
        .Text(FText::FromString(m_objFolderPath))
        ]
        ];
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
        .Text(LOCTEXT("Block3", "3.Configure"))
        .IsReadOnly(true)
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]

    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("LOD設定")))
        ]


    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("出力モード")))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SComboBox<TSharedPtr<FString>>)
            .OptionsSource(&this->m_outputModeArray)
        .OnSelectionChanged_Raw(this, &PlateauWindow::onSelectOutputMode)
        .OnGenerateWidget_Lambda([](TSharedPtr<FString> value)->TSharedRef<SWidget>
            {
                return SNew(STextBlock).Text(FText::FromString(*value));
            })
        [
            SNew(STextBlock)
            .Text_Raw(this, &PlateauWindow::onGetBuildOutputMode)
        ]
        ]
        ]

    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString(TEXT("出力LOD(MAX)1-3"))))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SSpinBox<int>)
            .Value(m_buildMaxLOD)
        .MinSliderValue(1)
        .MaxSliderValue(3)
        .OnValueChanged_Raw(this, &PlateauWindow::onBuildMaxLODChanged)
        ]
        ]

    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString(TEXT("出力LOD(MIN)1-3"))))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SSpinBox<int>)
            .Value(m_buildMinLOD)
        .MinSliderValue(1)
        .MaxSliderValue(3)
        .OnValueChanged_Raw(this, &PlateauWindow::onBuildMinLODChanged)
        ]
        ]

        ];
#pragma endregion

#pragma region Convert
    scrollBox->AddSlot()
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [
            SNew(SEditableTextBox)
            .Padding(FMargin(3, 3, 0, 3))
        .Text(LOCTEXT("Block4", "4.Convert"))
        .IsReadOnly(true)
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
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
        .Text(LOCTEXT("Button4", "Convert"))
        ]
        ]

        ];

#pragma endregion

    window.Pin()->SetContent(scrollBox);
}

FReply PlateauWindow::onBtnSelectGmlFileClicked() {
    void* windowHandle = m_myWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    FString dialogTitle = FString("Select gml File");
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
        .Title(LOCTEXT("ChooseImportRootContentPath", "Choose Location for importing the scene content"));

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
            UdxSubFolder::veg().name(),
            UdxSubFolder::frn().name(),
            UdxSubFolder::dem().name()
        };
        for (int i = 0; i < m_existFeatures.Num(); i++) {
            if (!m_existFeatures[i])continue;
            if (!m_selectFeature[selectIndex]) {
                selectIndex++;
                continue;
            }
            for (auto subfolder : *m_subFolders) {
                if (i < m_existFeatures.Num() - 1 && subfolder.name() == checkSubFolder[i]) {
                    m_filteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), subfolder);
                }
                else if (i == m_existFeatures.Num() - 1 && checkSubFolder.Find(subfolder.name()) == INDEX_NONE) {
                    m_filteredCollection.copyFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")), subfolder);
                }
            }
            selectIndex++;
        }
        m_filteredCollection.copyCodelistFiles(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")));

        //setting convert option
        MeshConverter meshConverter;
        MeshConvertOptions options;
        options.unit_scale = 0.01;
        options.mesh_axes = AxesConversion::NWU;
        options.max_lod = m_buildMaxLOD;
        options.min_lod = m_buildMinLOD;
        switch (m_buildOutputIndex) {
        case 0:
            options.export_lower_lod = true;
            break;
        case 1:
            options.export_lower_lod = false;
            break;
        default:
            break;
        }
        meshConverter.setOptions(options);

        //convert gml file to obj file
        const citygml::ParserParams params;
        TArray<std::string> gmlFilePathArray;
        TArray<FString> objFilePahtArray;

        for (auto gmlFile : std::filesystem::recursive_directory_iterator(TCHAR_TO_ANSI(*(FPaths::ProjectContentDir() + "PLATEAU/")))) {
            if (gmlFile.path().extension() == ".gml") {
                gmlFilePathArray.Add(gmlFile.path().string());
            }
        }

        for (auto objFile : std::filesystem::recursive_directory_iterator(TCHAR_TO_ANSI(*m_objFolderPath))) {
            if (objFile.path().extension() == ".obj") {
                objFilePahtArray.Add(FString(objFile.path().string().c_str()));
            }
        }

        for (auto gmlFilePath : gmlFilePathArray) {
            const auto cityModel = citygml::load(gmlFilePath, params, nullptr);
            if (cityModel == nullptr) {
                throw std::runtime_error(std::string("Failed to load") + gmlFilePath);
            }
            meshConverter.convert(TCHAR_TO_UTF8(*m_objFolderPath), gmlFilePath,
                cityModel, nullptr);
        }

        TSharedRef<SDlgPickPath> PickContentPathDlg =
            SNew(SDlgPickPath)
            .Title(LOCTEXT("ChooseImportRootContentPath", "Choose Location for importing the scene content"));

        if (PickContentPathDlg->ShowModal() == EAppReturnType::Cancel) {
            return FReply::Handled();
        }

        FString path = PickContentPathDlg->GetPath().ToString();

        PLATEAUFileUtils::ImportFbx(objFilePahtArray, path);

        return FReply::Handled();
    }
    catch (std::exception& e) {
        const auto errorLog = ANSI_TO_TCHAR(e.what());
        UE_LOG(LogTemp, Warning, TEXT("%s"), errorLog);
        return FReply::Handled();
    }
}

FReply PlateauWindow::onBtnAllRegionSelectClicked() {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        m_selectRegion[i] = true;
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

FReply PlateauWindow::onBtnAllRegionRelieveClicked() {
    for (int i = 0; i < m_selectRegion.Num(); i++) {
        m_selectRegion[i] = false;
    }
    checkRegionMesh();
    updatePlateauWindow(m_myWindow.Pin());
    return FReply::Handled();
}

void PlateauWindow::onToggleCbSelectRegion(ECheckBoxState checkState, int num) {
    m_selectRegion[num] = (checkState == ECheckBoxState::Checked);
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

void PlateauWindow::onSelectOutputMode(TSharedPtr<FString> newSelection, ESelectInfo::Type selectInfo) {
    m_buildOutputIndex = m_outputModeArray.Find(newSelection);
}

FText PlateauWindow::onGetBuildOutputMode()const {
    return FText::FromString(*m_outputModeArray[m_buildOutputIndex]);
}

void PlateauWindow::onBuildMaxLODChanged(int value) {
    m_buildMaxLOD = value;
}

void PlateauWindow::onBuildMinLODChanged(int value) {
    m_buildMinLOD = value;
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
    int selectFeatureSize = 0;
    if (targetMeshCodes.size() != 0) {

        m_filteredCollection = UdxFileCollection::filter(m_collection, targetMeshCodes);
        m_subFolders = m_filteredCollection.getSubFolders();

        for (auto subfolder : *m_subFolders) {
            if (subfolder.name() == "bldg") {
                m_existFeatures[0] = true;
            }
            else if (subfolder.name() == "tran") {
                m_existFeatures[1] = true;
            }
            else if (subfolder.name() == "veg") {
                m_existFeatures[2] = true;
            }
            else if (subfolder.name() == "frn") {
                m_existFeatures[3] = true;
            }
            else if (subfolder.name() == "dem") {
                m_existFeatures[4] = true;
            }
            else {
                m_existFeatures[5] = true;
            }
        }
        for (bool flg : m_existFeatures) {
            if (flg) selectFeatureSize++;
        }
        m_selectFeature.Init(false, selectFeatureSize);
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