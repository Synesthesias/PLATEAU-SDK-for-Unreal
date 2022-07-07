// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class PLATEAUSDK_API PlateauWindow : public TSharedFromThis<PlateauWindow>
{
public:
    PlateauWindow();

    void startup();
    void shutdown();


private:
    TSharedPtr<FExtender> m_extender;
    TWeakPtr<SWindow> m_rootWindow;
    TWeakPtr<SWindow> m_myWindow;
    FString m_gmlFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    FString m_gmlCopyPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() + "PLATEAU/");
    FString m_objFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    int32 m_axesConversionIndex = 1;
    TArray<TSharedPtr<FString>> m_axesConversions;
    bool m_cbOptimize;
    bool m_cbMergeMesh;

    void onWindowMenuBarExtension(FMenuBarBuilder& menuBarBuilder);
    void onPulldownMenuExtension(FMenuBuilder& menuBuilder);
    void showPlateauWindow();
    void onMainFrameLoad(TSharedPtr<SWindow> inRootWindow, bool isNewProjectWindow);
    void showGML2OBJWindow(TWeakPtr<SWindow> window);

    FReply onBtnSelectGmlFileClicked();
    FReply onBtnSelectObjDestinationClicked();
    FReply onBtnConvertClicked();
    void onToggleCbOptimize(ECheckBoxState checkState);
    void onToggleCbMergeMesh(ECheckBoxState checkState);
    void onSelectAxesConversion(TSharedPtr<FString> newSelection, ESelectInfo::Type selectInfo);
    FText onGetAxesConversion() const;
    void copyGmlFiles(FString path);
};
