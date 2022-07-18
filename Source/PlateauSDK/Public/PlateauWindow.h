// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <vector>

#include "plateau/udx/mesh_code.h"
#include "plateau/udx/udx_file_collection.h"


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
    FString m_gmlFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    FString m_objFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    std::shared_ptr<std::vector<MeshCode>> m_meshCodes;
    std::shared_ptr<std::vector<UdxSubFolder>> m_subFolders;
    TArray<TSharedPtr<FString>> m_outputModeArray;
    int m_buildOutputIndex = 0;
    int m_buildMaxLOD = 3;
    int m_buildMinLOD = 1;
    TArray<FString> m_Features = {
        TEXT("建築物"),
        TEXT("道路"),
        TEXT("植生"),
        TEXT("都市設備"),
        TEXT("起伏"),
        TEXT("その他")
    };
    TArray<bool> m_selectRegion; //選択された地域メッシュ
    TArray<bool> m_existFeatures; //選択された地域に含まれる地物（その他あり）
    TArray<bool> m_selectFeature; //選択された地物（その他あり）
    bool m_gmlFileSelected = false;
    int m_selectFeatureSize;
    UdxFileCollection m_collection;
    UdxFileCollection m_filteredCollection;

    void onWindowMenuBarExtension(FMenuBarBuilder& menuBarBuilder);
    void onPulldownMenuExtension(FMenuBuilder& menuBuilder);
    void showPlateauWindow();
    void onMainFrameLoad(TSharedPtr<SWindow> inRootWindow, bool isNewProjectWindow);
    void updatePlateauWindow(TWeakPtr<SWindow> window);

    FReply onBtnSelectGmlFileClicked();
    FReply onBtnSelectObjDestinationClicked();
    FReply onBtnConvertClicked();
    FReply onBtnAllRegionSelectClicked();
    FReply onBtnAllRegionRelieveClicked();
    void onToggleCbSelectRegion(ECheckBoxState checkState, int num);
    FReply onBtnAllFeatureSelectClicked();
    FReply onBtnAllFeatureRelieveClicked();
    void onToggleCbSelectFeature(ECheckBoxState checkState, int index);
    void onSelectOutputMode(TSharedPtr<FString> newSelection, ESelectInfo::Type selectInfo);
    FText onGetBuildOutputMode() const;
    void onBuildMaxLODChanged(int value);
    void onBuildMinLODChanged(int value);
    void checkRegionMesh();
};
