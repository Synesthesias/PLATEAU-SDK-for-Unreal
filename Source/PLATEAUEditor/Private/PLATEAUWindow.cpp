// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUWindow.h"

#include "LevelEditor.h"
#include "PLATEAUEditor.h"
#include "SPLATEAUImportPanel.h"
#include "SPLATEAUMainTab.h"
#include "SPLATEAUExportPanel.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Dialogs/DlgPickPath.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Framework/Docking/LayoutExtender.h"
#include "Widgets/SPLATEAUFilteringPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/PLATEAUSDKEditorUtilityWidget.h"

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
    const auto EditorUtilityWidget = LoadObject<UBlueprint>(nullptr, WidgetPath, nullptr, LOAD_EditorOnly, nullptr);
    if (!IsValid(EditorUtilityWidget)) {
        UE_LOG(LogTemp, Warning, TEXT("Missing Expected widget class at : %s"), WidgetPath);
        const FText Title = LOCTEXT("Warning", "警告");
        const FText DialogText = LOCTEXT("WidgetError", "ウィンドウを開けませんでした。プラグインが破損している可能性があります。");
        FMessageDialog::Open(EAppMsgType::Ok, DialogText, &Title);
        const auto SpawnedTab = SNew(SDockTab).ShouldAutosize(false).TabRole(NomadTab);
        return SpawnedTab;
    }
    
    const TSubclassOf<UPLATEAUSDKEditorUtilityWidget> WidgetClass{EditorUtilityWidget->GeneratedClass};
    const auto& World = GEditor->GetEditorWorldContext().World();
    const auto& PLATEAUSDKEditorUtilityWidget = CreateWidget<UPLATEAUSDKEditorUtilityWidget>(World, WidgetClass);
    
    const auto& ExtentEditor = IPLATEAUEditorModule::Get().GetExtentEditor();
    ExtentEditor->SetPLATEAUSDKEditorUtilityWidget(PLATEAUSDKEditorUtilityWidget);
    
    const auto SpawnedTab = SNew(SDockTab).ShouldAutosize(false).TabRole(NomadTab);
    SpawnedTab->SetContent(PLATEAUSDKEditorUtilityWidget->TakeWidget());
    return SpawnedTab;
}

void FPLATEAUWindow::ConstructTab() {    
    TSharedRef<FGlobalTabmanager> GTabManager = FGlobalTabmanager::Get();
    GTabManager->RegisterNomadTabSpawner(TabID, FOnSpawnTab::CreateRaw(this, &FPLATEAUWindow::SpawnTab))
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

TSharedPtr<SVerticalBox> FPLATEAUWindow::Show() {
    TabReference = SNew(SPLATEAUMainTab, Style.ToSharedRef());
    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()[
            TabReference.ToSharedRef()
        ]
        + SVerticalBox::Slot()[
            //インポート、編集、エクスポートそれぞれのスクロール部分
            //TODO:編集画面のUIが出来次第組み込む
            SNew(SOverlay)
                + SOverlay::Slot()
                .HAlign(HAlign_Fill)
                .VAlign(VAlign_Top)[
                    SNew(SScrollBox)
                        .Visibility_Lambda([=]() {
                        if (TabReference->IsCurrentIndex(1))
                        return EVisibility::Visible;
                        else
                            return EVisibility::Collapsed;
                            })
                        + SScrollBox::Slot()[
                            SNew(SPLATEAUImportPanel, Style.ToSharedRef())
                        ]
                ]
                + SOverlay::Slot()
                                .HAlign(HAlign_Center)
                                .VAlign(VAlign_Top)[
                                    SNew(SScrollBox)
                                        .Visibility_Lambda([=]() {
                                        if (TabReference->IsCurrentIndex(2))
                                        return EVisibility::Visible;
                                        else
                                            return EVisibility::Collapsed;
                                            })
                                        + SScrollBox::Slot()[
                                            SNew(SPLATEAUFilteringPanel, Style.ToSharedRef())
                                        ]
                                ]
                                + SOverlay::Slot()
                                                .HAlign(HAlign_Center)
                                                .VAlign(VAlign_Top)[
                                                    SNew(SScrollBox)
                                                        .Visibility_Lambda([=]() {
                                                        if (TabReference->IsCurrentIndex(3))
                                                        return EVisibility::Visible;
                                                        else
                                                            return EVisibility::Collapsed;
                                                            })
                                                        + SScrollBox::Slot()[
                                                            SNew(SPLATEAUExportPanel, Style.ToSharedRef())
                                                        ]
                                                ]
        ];
}

#undef LEVEL_EDITOR_NAME
#undef LOCTEXT_NAMESPACE