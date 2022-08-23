// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Layout/SScrollBox.h"
#include "plateau/udx/udx_file_collection.h"
#include "CityModelImportData.h"
#include <memory>
#include <vector>


/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUGmlSelectPanel {
public:
    FPLATEAUGmlSelectPanel();
    void UpdateWindow(TWeakPtr<SWindow> MyWindow);

private:
    FString GmlFolderPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    bool GmlFileSelected = false;
    UdxFileCollection Collection;
    UdxFileCollection FilteredCollection;
    std::shared_ptr<std::vector<MeshCode>> MeshCodes;
    std::shared_ptr<std::vector<UdxSubFolder>> SubFolders;
    TArray<bool> SelectRegion; //選択された地域メッシュ
    TArray<MeshCode> SecondMesh;
    TArray<bool> ExistFeatures; //選択された地域に含まれる地物（その他あり）
    TArray<ECityModelPackage> Features = {
        ECityModelPackage::Building,
        ECityModelPackage::Road,
        ECityModelPackage::UrbanFacility,
        ECityModelPackage::Relief,
        ECityModelPackage::Vegetation,
        ECityModelPackage::Others
    };
    TArray<bool> SelectFeatures; //選択された地物（その他あり）

    TSharedRef<SVerticalBox> CreateSelectGmlFilePanel(TWeakPtr<SWindow> MyWindow);
    TSharedRef<SVerticalBox> CreateSelectRegionMesh();
    TSharedRef<SVerticalBox> CreateSelectFeatureMesh();
    FReply OnBtnSelectGmlFileClicked(TWeakPtr<SWindow> MyWindow);
    TArray<MeshCode> SortMeshCodes(TArray<MeshCode> MeshArray);
    FReply OnBtnAllSecondMeshSelectClicked();
    FReply OnBtnAllSecondMeshRelieveClicked();
    FReply OnBtnThirdMeshSelectClicked(std::string SecondMeshName);
    FReply OnBtnThirdMeshRelieveClicked(std::string SecondMeshName);
    void OnToggleCbSelectRegion(ECheckBoxState CheckState, int Num, std::string MeshName);
    void CheckRegionMesh();
    FReply OnBtnAllFeatureSelectClicked();
    FReply OnBtnAllFeatureRelieveClicked();
    void OnToggleCbSelectFeature(ECheckBoxState CheckState, int Index);
};
