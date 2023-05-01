// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUWindow
{
public:
    FPLATEAUWindow(const TSharedRef<class FPLATEAUEditorStyle>& InStyle);

    void Startup();
    void Shutdown();

private:
    TSharedPtr<FExtender> Extender;
    TWeakPtr<SWindow> RootWindow;
    TWeakPtr<SWindow> MyWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    TSharedPtr<class SPLATEAUMainTab> TabReference;

    static const FName TabID;

    TSharedPtr<SVerticalBox> Show();

    void OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
    void OnPulldownMenuExtension(FMenuBuilder& MenuBuilder);
    void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow);
    TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs);
    void ConstructTab();
};
