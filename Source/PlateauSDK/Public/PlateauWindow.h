// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <memory>

/**
 * 
 */
class PLATEAUSDK_API PlateauWindow : public TSharedFromThis<PlateauWindow>
{
public:
	PlateauWindow();


    void startup();
    void shutdown();


private:
    TSharedPtr<FExtender> extender_;
    TWeakPtr<SWindow> rootWindow_;
    TWeakPtr<SWindow> myWindow_;
    FString gml_file_path_ = FPaths::ProjectContentDir();
    FString obj_file_path_ = FPaths::ProjectContentDir();
    int32 axes_conversion_index_ = 1;
    TArray<TSharedPtr<FString>> axes_conversions_;
    bool cb_optimize_;
    bool cb_merge_mesh_;

    void onWindowMenuBarExtension(FMenuBarBuilder& menu_bar_builder);
    void onPulldownMenuExtension(FMenuBuilder& menu_builder);
    void showPlateauWindow();
    void onMainFrameLoad(TSharedPtr<SWindow> in_root_window, bool is_new_project_window);
    void showGML2OBJWindow(TWeakPtr<SWindow> window);

    FReply onBtnSelectGmlFileClicked();
    FReply onBtnSelectObjDestinationClicked();
    FReply onBtnConvertClicked();
    void onToggleCbOptimize(ECheckBoxState check_state);
    void onToggleCbMergeMesh(ECheckBoxState check_state);
    void onSelectAxesConversion(TSharedPtr<FString> new_selection, ESelectInfo::Type select_info);
    FText onGetAxesConversion() const;
};
