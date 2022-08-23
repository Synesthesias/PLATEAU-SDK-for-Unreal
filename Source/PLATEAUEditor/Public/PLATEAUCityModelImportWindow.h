// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGmlSelectPanel.h"
#include "PLATEAUMeshConvertSettingsPanel.h"


/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUCityModelImportWindow
{
public:
    void Startup();
    void Shutdown();
    void UpdateFeaturesInfo(TArray<bool> ExistArray, TArray<bool> SelectArray, UdxFileCollection Collection);
    static FPLATEAUCityModelImportWindow* GetInstance();
    FPLATEAUMeshConvertSettingsPanel& GetMeshConvertSettingsPanel();

private:
    static FPLATEAUCityModelImportWindow* CityModelImportWindow;

    FPLATEAUGmlSelectPanel GmlSelectPanel;
    FPLATEAUMeshConvertSettingsPanel MeshConvertSettingsPanel;
    TSharedPtr<FExtender> Extender;
    TWeakPtr<SWindow> RootWindow;
    TWeakPtr<SWindow> MyWindow;

    void OnWindowMenuBarExtension(FMenuBarBuilder& MenuBarBuilder);
    void OnPulldownMenuExtension(FMenuBuilder& MenuBuilder);
    void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool IsNewProjectWindow);
    void ShowPlateauWindow();
};
