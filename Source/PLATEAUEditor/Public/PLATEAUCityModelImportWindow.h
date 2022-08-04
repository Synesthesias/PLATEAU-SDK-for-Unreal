// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGmlSelectPanel.h"

/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUCityModelImportWindow
{
public:
    FPLATEAUCityModelImportWindow();

    void Startup();
    void Shutdown();

private:
    FPLATEAUGmlSelectPanel GmlSelectPanelInstance;
    TSharedPtr<FExtender> Extender;
    TWeakPtr<SWindow> RootWindow;
    TWeakPtr<SWindow> MyWindow;

    void OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
    void OnPulldownMenuExtension(FMenuBuilder& MenuBuilder);
    void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow);
    void ShowPlateauWindow();
};
