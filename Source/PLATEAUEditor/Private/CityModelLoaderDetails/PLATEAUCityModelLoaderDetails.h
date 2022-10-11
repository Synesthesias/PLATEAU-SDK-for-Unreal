// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "PLATEAUFeatureLoadSettingsPropertyRow.h"
#include "IDetailCustomization.h"
#include "PLATEAUCityModelLoader.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

/**
 * APLATEAUCityModelLoaderの詳細パネルをカスタマイズします。
 */
class FPLATEAUCityModelLoaderDetails : public IDetailCustomization {
public:
    /**
     * @brief FPLATEAUCityModelLoaderDetailsのインスタンスを生成します。
     */
    static TSharedRef<IDetailCustomization> MakeInstance();

    /** IDetailCustomization インターフェース **/
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
    
private:
    TSharedPtr<IPropertyHandle> BuildingPlacementModeProperty;
    TSharedPtr<IPropertyHandle> BuildingLODProperty;

    // UIの内部状態
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;

    TMap<ECityModelPackage, FPLATEAUFeatureLoadSettingsPropertyRow> FeaturePlacementRows;
};
