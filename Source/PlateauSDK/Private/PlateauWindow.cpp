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
#include "citygml/citymodel.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"

#include "obj_writer.h"
#include "citygml/citygml.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPlateauSDKModule"


PlateauWindow::PlateauWindow() {
    axes_conversions_.Empty();
    axes_conversions_.Add(MakeShareable(new FString(TEXT("WNU"))));
    axes_conversions_.Add(MakeShareable(new FString(TEXT("RUF"))));
}

void PlateauWindow::onMainFrameLoad(TSharedPtr<SWindow> in_root_window, bool is_new_project_window) {
    if ((!is_new_project_window) && (in_root_window.IsValid())) {
        rootWindow_ = in_root_window;
    }
}

void PlateauWindow::onWindowMenuBarExtension(FMenuBarBuilder& menu_bar_builder) {
    menu_bar_builder.AddPullDownMenu(
        LOCTEXT("MenuBarTitle", "Plateau"),
        LOCTEXT("MenuBarToolkit", "Open Plateau menu."),
        FNewMenuDelegate::CreateRaw(this, &PlateauWindow::onPulldownMenuExtension)
    );
}

void PlateauWindow::onPulldownMenuExtension(FMenuBuilder& menu_builder) {
    menu_builder.AddMenuEntry(
        LOCTEXT("MenuTitle", "Model File Converter Window"),
        LOCTEXT("PulldownMenuToolTip", "Show Model File Converter Window."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &PlateauWindow::showPlateauWindow)));
}

void PlateauWindow::showPlateauWindow() {
    if (!myWindow_.IsValid()) {
        TSharedPtr<SWindow> window = SNew(SWindow)
            .Title(LOCTEXT("Model File Converter Window", "Model File Converter"))
            .ClientSize(FVector2D(300.f, 300.f));
        myWindow_ = TWeakPtr<SWindow>(window);

        if (rootWindow_.IsValid()) {
            FSlateApplication::Get().AddWindowAsNativeChild(
                window.ToSharedRef(), rootWindow_.Pin().ToSharedRef());
        }
        showGML2OBJWindow(myWindow_);
    }
    myWindow_.Pin()->BringToFront();
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
            //    + SVerticalBox::Slot()
            //.AutoHeight()
            //[
            //    SNew(STextBlock)
            //    .Text(LOCTEXT("Annotation1", "Assetsフォルダ以外のファイルも指定できます。"))
            //]

        +SVerticalBox::Slot()
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
            .Text(FText::FromString(gml_file_path_))
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
            .Text(FText::FromString(obj_file_path_))
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
            .OptionsSource(&this->axes_conversions_)
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
    void* window_handle = myWindow_.Pin()->GetNativeWindow()->GetOSWindowHandle();
    TArray<FString> out_file_names;
    FString dialog_title = FString("Select gml File");
    FString default_path = FString(FPaths::ProjectContentDir());
    FString default_file;
    FString file_types = FString("gml | *.gml");
    uint32 flags = 0;

    IDesktopPlatform* desktop_platform = FDesktopPlatformModule::Get();

    bool flg = desktop_platform->OpenFileDialog(
        window_handle,
        dialog_title,
        default_path,
        default_file,
        file_types,
        flags,
        out_file_names);

    if (out_file_names.Num() != 0) {
        gml_file_path_ = out_file_names.Pop();
        showGML2OBJWindow(myWindow_.Pin());
    }

    return FReply::Handled();
}

FReply PlateauWindow::onBtnSelectObjDestinationClicked() {
    void* window_handle = myWindow_.Pin()->GetNativeWindow()->GetOSWindowHandle();
    TArray<FString> out_file_names;
    FString dialog_title = FString("Select destination");
    FString default_path = FString(FPaths::ProjectContentDir());
    FString default_file = TEXT("exported.obj");
    FString file_types = FString("obj | *.obj");
    uint32 flags = 0;

    IDesktopPlatform* desktop_platform = FDesktopPlatformModule::Get();

    bool flg = desktop_platform->SaveFileDialog(
        window_handle,
        dialog_title,
        default_path,
        default_file,
        file_types,
        flags,
        out_file_names);

    if (out_file_names.Num() != 0) {
        obj_file_path_ = out_file_names.Pop();
        showGML2OBJWindow(myWindow_.Pin());
    }
    return FReply::Handled();
}

FReply PlateauWindow::onBtnConvertClicked() {
    try {
        ObjWriter objWriter;
        AxesConversion axes;

        if (!axes_conversions_[axes_conversion_index_]->Compare(FString("WNU"), ESearchCase::CaseSensitive)) {
            axes = AxesConversion::WNU;
        } else {
            axes = AxesConversion::RUF;
        }
        const citygml::ParserParams params;
        const auto CityModel = citygml::load(TCHAR_TO_UTF8(*gml_file_path_), params, nullptr);
        if (CityModel == nullptr)
        {
            throw std::runtime_error(std::string("Failed to load") + TCHAR_TO_UTF8(*gml_file_path_));
        }

        objWriter.setValidReferencePoint(*CityModel);
        objWriter.setMergeMeshFlg(cb_merge_mesh_);
        objWriter.setDestAxes(axes);
        objWriter.write(TCHAR_TO_UTF8(*obj_file_path_), *CityModel, TCHAR_TO_UTF8(*gml_file_path_));
        
        return FReply::Handled();
    }
    catch (std::exception& e) {
        const auto errorLog = ANSI_TO_TCHAR(e.what());
        UE_LOG(LogTemp, Warning, TEXT("%s"), errorLog);
        return FReply::Handled();
    }
}

void PlateauWindow::onToggleCbOptimize(ECheckBoxState check_state) {
    cb_optimize_ = (check_state == ECheckBoxState::Checked);
}

void PlateauWindow::onToggleCbMergeMesh(ECheckBoxState check_state) {
    cb_merge_mesh_ = (check_state == ECheckBoxState::Checked);
}

void PlateauWindow::startup() {
    FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
    extender_ = MakeShared<FExtender>();
    if (extender_.IsValid()) {
        extender_->AddMenuBarExtension(
            "Help",
            EExtensionHook::After,
            nullptr,
            FMenuBarExtensionDelegate::CreateRaw(this, &PlateauWindow::onWindowMenuBarExtension)
        );
    }
    const auto menuExtensibilityManager = levelEditorModule.GetMenuExtensibilityManager();
    if (menuExtensibilityManager.IsValid()) {
        menuExtensibilityManager->AddExtender(extender_);
    }

    IMainFrameModule& mainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
    mainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &PlateauWindow::onMainFrameLoad);
}

void PlateauWindow::shutdown() {
    if (extender_.IsValid() && FModuleManager::Get().IsModuleLoaded(LEVEL_EDITOR_NAME)) {
        FLevelEditorModule& levelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
        const auto menuExtensibilityManager = levelEditorModule.GetMenuExtensibilityManager();
        if (menuExtensibilityManager.IsValid()) {
            menuExtensibilityManager->RemoveExtender(extender_);
        }
    }

    if (FModuleManager::Get().IsModuleLoaded("MainFrame")) {
        IMainFrameModule& mainFrameModule =
            FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
        mainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
    }
}

FText PlateauWindow::onGetAxesConversion() const {
    return FText::FromString(*axes_conversions_[axes_conversion_index_]);
}

void PlateauWindow::onSelectAxesConversion(TSharedPtr<FString> new_selection, ESelectInfo::Type select_info) {
    axes_conversion_index_ = axes_conversions_.Find(new_selection);
}
