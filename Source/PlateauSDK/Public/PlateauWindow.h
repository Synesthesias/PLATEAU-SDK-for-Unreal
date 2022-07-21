// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <vector>

#include "CityMapMetadata.h"
#include "plateau/udx/mesh_code.h"
#include "plateau/udx/udx_file_collection.h"
#include "plateau/mesh/mesh_convert_options.h"

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
    FString m_gmlFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    FString m_objFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    std::shared_ptr<std::vector<MeshCode>> m_meshCodes;
    std::shared_ptr<std::vector<UdxSubFolder>> m_subFolders;
    TArray<TSharedPtr<FString>> m_outputModeArray;
    TArray<ECityModelPackage> m_Features = {
        ECityModelPackage::Building,
        ECityModelPackage::Road,
        ECityModelPackage::UrbanFacility,
        ECityModelPackage::Relief,
        ECityModelPackage::Vegetation,
        ECityModelPackage::Others
    };
    TArray<bool> m_selectRegion; //選択された地域メッシュ
    TArray<bool> m_existFeatures; //選択された地域に含まれる地物（その他あり）
    TArray<bool> m_selectFeature; //選択された地物（その他あり）
    int m_selectFeatureSize;
    TArray<MeshCode> m_secondMesh;
    bool m_gmlFileSelected = false;
    UdxFileCollection m_collection;
    UdxFileCollection m_filteredCollection;
    bool m_includeAppearance = true;
    MeshGranularity m_meshGranularity = MeshGranularity::PerPrimaryFeatureObject;

    void onWindowMenuBarExtension(FMenuBarBuilder& menuBarBuilder);
    void onPulldownMenuExtension(FMenuBuilder& menuBuilder);
    void showPlateauWindow();
    void onMainFrameLoad(TSharedPtr<SWindow> inRootWindow, bool isNewProjectWindow);
    void updatePlateauWindow(TWeakPtr<SWindow> window);

    FReply onBtnSelectGmlFileClicked();
    FReply onBtnSelectObjDestinationClicked();
    FReply onBtnConvertClicked();
    FReply onBtnAllSecondMeshSelectClicked();
    FReply onBtnAllSecondMeshRelieveClicked();
    FReply onBtnThirdMeshSelectClicked(std::string secondMesh);
    FReply onBtnThirdMeshRelieveClicked(std::string secondMesh);
    void onToggleCbSelectRegion(ECheckBoxState checkState, int num, std::string meshName);
    FReply onBtnAllFeatureSelectClicked();
    FReply onBtnAllFeatureRelieveClicked();
    void onToggleCbSelectFeature(ECheckBoxState checkState, int index);
    void checkRegionMesh();

    TArray<MeshCode> sortMeshCodes(TArray<MeshCode> meshArray);

    TSharedRef<SVerticalBox> CreateLODSettingsPanel(ECityModelPackage Package);

    TMap<ECityModelPackage, MeshConvertOptions> MeshConvertOptionsMap;
};
