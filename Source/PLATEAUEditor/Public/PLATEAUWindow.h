// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUWindow
{
public:
    FPLATEAUWindow();

    void Startup();
    void Shutdown();

private:
    TSharedPtr<FExtender> Extender;
    TWeakPtr<SWindow> RootWindow;
    TWeakPtr<SWindow> MyWindow;

    void Show();

    void OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
    void OnPulldownMenuExtension(FMenuBuilder& MenuBuilder);
    void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow);
};
