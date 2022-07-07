// Fill out your copyright notice in the Description page of Project Settings.


#include "PlateauWindow.h"

#include "LevelEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Templates/SharedPointer.h"
#include "IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "AssetTools/Private/AssetTools.h"
#include <direct.h>

#include "citygml/citygml.h"
#include "plateau/io/mesh_converter.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPlateauSDKModule"


PlateauWindow::PlateauWindow() {
    m_axesConversions.Empty();
    m_axesConversions.Add(MakeShareable(new FString(TEXT("WNU"))));
    m_axesConversions.Add(MakeShareable(new FString(TEXT("RUF"))));
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
        showGML2OBJWindow(m_myWindow);
    }
    m_myWindow.Pin()->BringToFront();
}

void PlateauWindow::showGML2OBJWindow(TWeakPtr<SWindow> window) {
    window.Pin()->SetContent(
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [
            SNew(SScrollBox)
#pragma region 1_Select_gml_File
            + SScrollBox::Slot()
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
            .Text(FText::FromString(m_gmlFilePath))
        ]
        ]
#pragma  endregion 

#pragma region 2_Select_obj_File_Destination
    + SScrollBox::Slot()
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
        ]
#pragma endregion

#pragma region 3_Configure
    + SScrollBox::Slot()
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
        .Padding(0, 0, 0, 3)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("text3-1", "Optimize"))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SCheckBox)
            .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbOptimize)
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 3)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
        [
            SNew(STextBlock)
            .Text(LOCTEXT("text3-2", "Merge Mesh"))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SCheckBox)
            .OnCheckStateChanged_Raw(this, &PlateauWindow::onToggleCbMergeMesh)
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
            .Text(LOCTEXT("text3-3", "Axes Conversion"))
        ]
    + SHorizontalBox::Slot()
        [
            SNew(SComboBox<TSharedPtr<FString>>)
            .OptionsSource(&this->m_axesConversions)
        .OnSelectionChanged_Raw(this, &PlateauWindow::onSelectAxesConversion)
        .OnGenerateWidget_Lambda([](TSharedPtr<FString> value)->TSharedRef<SWidget>
            {
                return SNew(STextBlock).Text(FText::FromString(*value));
            })
        [
            SNew(STextBlock)
            .Text_Raw(this, &PlateauWindow::onGetAxesConversion)
        ]
        ]
        ]
        ]
#pragma endregion

#pragma region 4_Convert
    + SScrollBox::Slot()
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

        ]

#pragma endregion
        ]

    );
}

FReply PlateauWindow::onBtnSelectGmlFileClicked() {
    void* windowHandle = m_myWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    TArray<FString> outFileNames;
    FString dialogTitle = FString("Select gml File");
    FString defaultPath = FString(FPaths::ProjectContentDir());
    FString defaultFile;
    FString fileTypes = FString("gml | *.gml");
    uint32 flags = 0;

    IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();

    bool flg = desktopPlatform->OpenFileDialog(
        windowHandle,
        dialogTitle,
        defaultPath,
        defaultFile,
        fileTypes,
        flags,
        outFileNames);

    if (outFileNames.Num() != 0) {
        m_gmlFilePath = FPaths::ConvertRelativePathToFull(outFileNames.Pop());
        showGML2OBJWindow(m_myWindow.Pin());
    }

    return FReply::Handled();
}

FReply PlateauWindow::onBtnSelectObjDestinationClicked() {
    void* window_handle = m_myWindow.Pin()->GetNativeWindow()->GetOSWindowHandle();
    FString dialogTitle = FString("Select destination");
    FString defaultPath = FString(FPaths::ProjectContentDir());
    FString outFolderName;

    IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();

    bool flgs = desktopPlatform->OpenDirectoryDialog(
        window_handle,
        dialogTitle,
        defaultPath,
        outFolderName);

    if (!outFolderName.IsEmpty()) {
        m_objFolderPath = outFolderName;
        showGML2OBJWindow(m_myWindow.Pin());
    }

    return FReply::Handled();
}

FReply PlateauWindow::onBtnConvertClicked() {
    try {
        struct stat statBuf;
        if (stat(TCHAR_TO_ANSI(*m_gmlCopyPath), &statBuf)) {
            if (_mkdir(TCHAR_TO_ANSI(*m_gmlCopyPath)) != 0) {
                UE_LOG(LogTemp, Warning, TEXT("make directory failed"));
            }
        }

        const citygml::ParserParams params;
        const auto cityModel = citygml::load(TCHAR_TO_UTF8(*m_gmlFilePath), params, nullptr);
        if (cityModel == nullptr)
        {
            throw std::runtime_error(std::string("Failed to load") + TCHAR_TO_UTF8(*m_gmlFilePath));
        }

        std::vector<std::string> convertedFiles;
        MeshConverter meshConverter;
        meshConverter.convert(TCHAR_TO_UTF8(*m_objFolderPath), TCHAR_TO_UTF8(*m_gmlFilePath),
            cityModel, nullptr);

        return FReply::Handled();
    }
    catch (std::exception& e) {
        const auto errorLog = ANSI_TO_TCHAR(e.what());
        UE_LOG(LogTemp, Warning, TEXT("%s"), errorLog);
        return FReply::Handled();
    }
}

void PlateauWindow::onToggleCbOptimize(ECheckBoxState checkState) {
    m_cbOptimize = (checkState == ECheckBoxState::Checked);
}

void PlateauWindow::onToggleCbMergeMesh(ECheckBoxState checkState) {
    m_cbMergeMesh = (checkState == ECheckBoxState::Checked);
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

FText PlateauWindow::onGetAxesConversion() const {
    return FText::FromString(*m_axesConversions[m_axesConversionIndex]);
}

void PlateauWindow::onSelectAxesConversion(TSharedPtr<FString> newSelection, ESelectInfo::Type selectInfo) {
    m_axesConversionIndex = m_axesConversions.Find(newSelection);
}
