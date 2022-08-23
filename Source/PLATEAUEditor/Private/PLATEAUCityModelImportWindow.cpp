// Fill out your copyright notice in the Description page of Project Settings.


#include "PLATEAUCityModelImportWindow.h"

#include "LevelEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Dialogs/DlgPickPath.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPLATEUEditorModule"


FPLATEAUCityModelImportWindow* FPLATEAUCityModelImportWindow::CityModelImportWindow = 0;

void FPLATEAUCityModelImportWindow::Startup() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
    Extender = MakeShared<FExtender>();
    if (Extender.IsValid()) {
        Extender->AddMenuBarExtension(
            "Help",
            EExtensionHook::After,
            nullptr,
            FMenuBarExtensionDelegate::CreateRaw(this, &FPLATEAUCityModelImportWindow::OnWindowMenuBarExtension)
        );
    }
    const auto MenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
    if (MenuExtensibilityManager.IsValid()) {
        MenuExtensibilityManager->AddExtender(Extender);
    }

    IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
    MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FPLATEAUCityModelImportWindow::OnMainFrameLoad);

}

void FPLATEAUCityModelImportWindow::Shutdown() {
    if (Extender.IsValid() && FModuleManager::Get().IsModuleLoaded(LEVEL_EDITOR_NAME)) {
        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
        const auto MenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
        if (MenuExtensibilityManager.IsValid()) {
            MenuExtensibilityManager->RemoveExtender(Extender);
        }
    }

    if (FModuleManager::Get().IsModuleLoaded("MainFrame")) {
        IMainFrameModule& MainFrameModule =
            FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
        MainFrameModule.OnMainFrameCreationFinished().RemoveAll(this);
    }
}

void FPLATEAUCityModelImportWindow::UpdateFeaturesInfo(TArray<bool> ExistArray, TArray<bool> SelectArray, UdxFileCollection Collection) {
    MeshConvertSettingsPanel.UpdateFeaturesInfo(ExistArray, SelectArray, Collection);
}

FPLATEAUCityModelImportWindow* FPLATEAUCityModelImportWindow::GetInstance() {
    if (CityModelImportWindow == NULL) {
        CityModelImportWindow = new FPLATEAUCityModelImportWindow();
    }
    return  CityModelImportWindow;
}

FPLATEAUMeshConvertSettingsPanel& FPLATEAUCityModelImportWindow::GetMeshConvertSettingsPanel() {
    return MeshConvertSettingsPanel;
}


void FPLATEAUCityModelImportWindow::OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder) {
    MenuBarBuilder.AddPullDownMenu(
        LOCTEXT("MenuBarTitle", "PLATEAU"),
        LOCTEXT("MenuBarToolkit", "PLATEAUメニューを開く."),
        FNewMenuDelegate::CreateRaw(this, &FPLATEAUCityModelImportWindow::OnPulldownMenuExtension)
    );
}

void FPLATEAUCityModelImportWindow::OnPulldownMenuExtension(FMenuBuilder& MenuBuilder) {
    MenuBuilder.AddMenuEntry(
        LOCTEXT("MenuTitle", "都市モデルをインポート"),
        LOCTEXT("PulldownMenuToolTip", "都市モデルインポート画面を開く."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FPLATEAUCityModelImportWindow::ShowPlateauWindow)));
}

void FPLATEAUCityModelImportWindow::OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow) {
    if ((!IsNewProjectWindow) && (InRootWindow.IsValid())) {
        RootWindow = InRootWindow;
    }
}

void FPLATEAUCityModelImportWindow::ShowPlateauWindow() {
    if (!MyWindow.IsValid()) {
        TSharedPtr<SWindow> Window = SNew(SWindow)
            .Title(LOCTEXT("Model File Converter Window", "都市モデルインポート画面"))
            .ClientSize(FVector2D(500.f, 400.f));
        MyWindow = TWeakPtr<SWindow>(Window);

        if (RootWindow.IsValid()) {
            FSlateApplication::Get().AddWindowAsNativeChild(
                Window.ToSharedRef(), RootWindow.Pin().ToSharedRef());
        }
        GmlSelectPanel.UpdateWindow(MyWindow);
    }
    MyWindow.Pin()->BringToFront();
}
