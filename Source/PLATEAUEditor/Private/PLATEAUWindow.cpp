// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUWindow.h"

#include "LevelEditor.h"
#include "SPLATEAUImportPanel.h"
#include "SPLATEAUMainTab.h"
#include "SPLATEAUExportPanel.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Dialogs/DlgPickPath.h"
#include "Widgets/Layout/SScrollBox.h"

#define LEVEL_EDITOR_NAME "LevelEditor"
#define LOCTEXT_NAMESPACE "FPLATEUEditorModule"

FPLATEAUWindow::FPLATEAUWindow(const TSharedRef<FPLATEAUEditorStyle>& InStyle)
    : Style(InStyle) {}

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
        FUIAction(FExecuteAction::CreateRaw(this, &FPLATEAUWindow::Show)));
}

void FPLATEAUWindow::OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow) {
    if ((!IsNewProjectWindow) && (InRootWindow.IsValid())) {
        RootWindow = InRootWindow;
    }
}

void FPLATEAUWindow::Show() {
    if (!MyWindow.IsValid()) {
        TabReference = SNew(SPLATEAUMainTab, Style.ToSharedRef());
        TSharedPtr<SWindow> Window = SNew(SWindow)
            .Title(LOCTEXT("PLATEAU SDK Window Title", "PLATEAU SDK"))
            .ClientSize(FVector2D(500.f, 700.f));
        Window->SetContent(
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight() [
                TabReference.ToSharedRef()
            ]
            + SVerticalBox::Slot()[
                //インポート、編集、エクスポートそれぞれのスクロール部分
                //TODO:編集画面のUIが出来次第組み込む
                SNew(SOverlay)
                    + SOverlay::Slot()
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Top) [
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
                    .VAlign(VAlign_Top) [
                        SNew(SScrollBox)
                        .Visibility_Lambda([=]() {
                            if (TabReference->IsCurrentIndex(2))
                                return EVisibility::Collapsed;
                            else
                                return EVisibility::Collapsed;
                        })
                        + SScrollBox::Slot()[
                            SNew(SPLATEAUImportPanel, Style.ToSharedRef())
                        ]
                    ]
                + SOverlay::Slot()
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Top) [
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
            ]
        );
        MyWindow = TWeakPtr<SWindow>(Window);

        if (RootWindow.IsValid()) {
            FSlateApplication::Get().AddWindowAsNativeChild(
                Window.ToSharedRef(), RootWindow.Pin().ToSharedRef());
        }

        //CityModelAddPanel->UpdateWindow(MyWindow);
    }
    MyWindow.Pin()->BringToFront();
}

#undef LEVEL_EDITOR_NAME
#undef LOCTEXT_NAMESPACE
