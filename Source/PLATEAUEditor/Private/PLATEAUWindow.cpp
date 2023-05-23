// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUWindow.h"

#include "EditorUtilityWidgetBlueprint.h"
#include "LevelEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Dialogs/DlgPickPath.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Framework/Docking/LayoutExtender.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPLATEUEditorModule"
constexpr TCHAR WidgetPath[] = TEXT("/PLATEAU-SDK-for-Unreal/EUW/MainWindow");

const FName FPLATEAUWindow::TabID(TEXT("PLATEAUWindow"));

FPLATEAUWindow::FPLATEAUWindow(const TSharedRef<FPLATEAUEditorStyle>& InStyle)
    : Style(InStyle) {
}

void FPLATEAUWindow::Startup() {
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LEVEL_EDITOR_NAME);
    Extender = MakeShared<FExtender>();
    if (Extender.IsValid()) {
        Extender->AddMenuBarExtension(
            "Help",
            EExtensionHook::After,
            nullptr,
            FMenuBarExtensionDelegate::CreateRaw(this, &FPLATEAUWindow::OnWindowMenuBarExtension)
        );
    }
    const auto MenuExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();
    if (MenuExtensibilityManager.IsValid()) {
        MenuExtensibilityManager->AddExtender(Extender);
    }

    IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
    MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FPLATEAUWindow::OnMainFrameLoad);
}

void FPLATEAUWindow::Shutdown() {
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
    TSharedRef<class FGlobalTabmanager> TabManager = FGlobalTabmanager::Get();
    TabManager->UnregisterNomadTabSpawner(TabID);
}

TSharedRef<SDockTab> FPLATEAUWindow::SpawnTab(const FSpawnTabArgs& TabSpawnArgs) {
    EditorUtilityWidgetBlueprint = LoadObject<UEditorUtilityWidgetBlueprint>(nullptr, WidgetPath);
    if (EditorUtilityWidgetBlueprint != nullptr) {
        return EditorUtilityWidgetBlueprint->SpawnEditorUITab(TabSpawnArgs);
    }

    const FText Title = LOCTEXT("WidgetWarning", "ウィジェット破損");
    const FText DialogText = LOCTEXT("WidgetWarningDetail", "PLATEAU SDKのウィジェットが破損しています。");
    FMessageDialog::Open(EAppMsgType::Ok, DialogText, &Title);
    return SNew(SDockTab).ShouldAutosize(false).TabRole(NomadTab);
}

UEditorUtilityWidget* FPLATEAUWindow::GetEditorUtilityWidget() const {
    return EditorUtilityWidgetBlueprint->GetCreatedWidget();
}

bool FPLATEAUWindow::CanSpawnTab(const FSpawnTabArgs& TabSpawnArgs) const {
    if (!EditorUtilityWidgetBlueprint.IsValid()) {
        return true;
    }
    return EditorUtilityWidgetBlueprint.IsValid() && EditorUtilityWidgetBlueprint->GetCreatedWidget() == nullptr;
}

void FPLATEAUWindow::ConstructTab() {    
    TSharedRef<FGlobalTabmanager> GTabManager = FGlobalTabmanager::Get();
    GTabManager->RegisterNomadTabSpawner(TabID, FOnSpawnTab::CreateRaw(this, &FPLATEAUWindow::SpawnTab), FCanSpawnTab::CreateRaw(this, &FPLATEAUWindow::CanSpawnTab))
        .SetDisplayName(FText::FromString(TEXT("PLATEAU SDK")));

    FLevelEditorModule* LevelEditorModule =
        FModuleManager::GetModulePtr<FLevelEditorModule>(
            FName(TEXT("LevelEditor")));
    if (LevelEditorModule) {
        LevelEditorModule->OnRegisterLayoutExtensions().AddLambda(
            [](FLayoutExtender& extender) {
                extender.ExtendLayout(
                    FTabId("PlacementBrowser"),
                    ELayoutExtensionPosition::After,
                    FTabManager::FTab(TabID, ETabState::OpenedTab));
            });
    }

    TSharedPtr<FTabManager> TabManager =
        LevelEditorModule
        ? LevelEditorModule->GetLevelEditorTabManager()
        : FGlobalTabmanager::Get();
    TabManager->TryInvokeTab(TabID);
}

void FPLATEAUWindow::OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder) {
    MenuBarBuilder.AddPullDownMenu(
        LOCTEXT("MenuBarTitle", "PLATEAU"),
        LOCTEXT("MenuBarToolkit", "PLATEAUメニューを開く."),
        FNewMenuDelegate::CreateRaw(this, &FPLATEAUWindow::OnPulldownMenuExtension)
    );
}

void FPLATEAUWindow::OnPulldownMenuExtension(FMenuBuilder& MenuBuilder) {
    MenuBuilder.AddMenuEntry(
        LOCTEXT("MenuTitle", "PLATEAU SDK"),
        LOCTEXT("PulldownMenuToolTip", "PLATEAU SDK画面を開く."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(this, &FPLATEAUWindow::ConstructTab)));
}

void FPLATEAUWindow::OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow) {
    if ((!IsNewProjectWindow) && (InRootWindow.IsValid())) {
        RootWindow = InRootWindow;
    }
}

#undef LEVEL_EDITOR_NAME
#undef LOCTEXT_NAMESPACE