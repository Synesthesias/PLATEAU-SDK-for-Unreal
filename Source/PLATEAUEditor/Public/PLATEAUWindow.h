// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"

class UEditorUtilityWidgetBlueprint;
class UEditorUtilityWidget;

class PLATEAUEDITOR_API FPLATEAUWindow {
public:
    FPLATEAUWindow();
    void Startup();
    void Shutdown();
    UEditorUtilityWidget* GetEditorUtilityWidget() const;
private:
    TSharedPtr<FExtender> Extender;
    TWeakPtr<SWindow> RootWindow;
    TWeakPtr<SWindow> MyWindow;
    TWeakObjectPtr<UEditorUtilityWidgetBlueprint> EditorUtilityWidgetBlueprint;
    TSharedPtr<class SPLATEAUMainTab> TabReference;

    static const FName TabID;

    TSharedPtr<SVerticalBox> Show();

    void OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
    void OnPulldownMenuExtension(FMenuBuilder& MenuBuilder);
    void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow);
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs);
    bool CanSpawnTab(const FSpawnTabArgs& TabSpawnArgs) const;
    void ConstructTab();
};
