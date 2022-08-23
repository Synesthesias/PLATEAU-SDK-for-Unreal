// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CityModelImportData.h"
#include "plateau/udx/udx_file_collection.h"
#include "plateau/mesh/mesh_convert_options.h"


/**
 *
 */
class PLATEAUEDITOR_API FPLATEAUMeshConvertSettingsPanel
{
public:
    FPLATEAUMeshConvertSettingsPanel();
    TSharedRef<SVerticalBox> CreateConvertSettingsPanel();
    void UpdateFeaturesInfo(TArray<bool> ExistArray, TArray<bool> SelectArray, UdxFileCollection Collection);

private:
    bool IncludeAppearance = true;
    MeshGranularity Granularity = MeshGranularity::PerPrimaryFeatureObject;
    TMap<ECityModelPackage, MeshConvertOptions> MeshConvertOptionsMap;
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
    UdxFileCollection FilteredCollection;

    TSharedRef<SVerticalBox> CreateLODSettingsPanel(ECityModelPackage Package);
    FReply OnBtnConvertClicked();
};
