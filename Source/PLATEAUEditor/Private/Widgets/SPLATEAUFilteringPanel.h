// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "PLATEAUInstancedCityModel.h"
#include <plateau/dataset/city_model_package.h>
#include <citygml/cityobject.h>

struct PLATEAUPackageLOD {
    int MaxLOD = 0;
    int MinLOD = 0;
};

class SPLATEAUFilteringPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUFilteringPanel) {}
    SLATE_ARGUMENT(TWeakPtr<class SWindow>, OwnerWindow)
        SLATE_END_ARGS()

private:
    TWeakPtr<SWindow> OwnerWindow;
    TSharedPtr<class FPLATEAUEditorStyle> Style;
    TArray<PLATEAUPackageLOD> TargetLODs;

    bool bShowMultiLOD = true;

    plateau::dataset::PredefinedCityModelPackage AvailablePackage = plateau::dataset::PredefinedCityModelPackage::None;
    plateau::dataset::PredefinedCityModelPackage EnablePackage = plateau::dataset::PredefinedCityModelPackage::Building | plateau::dataset::PredefinedCityModelPackage::CityFurniture |
        plateau::dataset::PredefinedCityModelPackage::DisasterRisk | plateau::dataset::PredefinedCityModelPackage::LandUse |
        plateau::dataset::PredefinedCityModelPackage::Relief | plateau::dataset::PredefinedCityModelPackage::Road |
        plateau::dataset::PredefinedCityModelPackage::Unknown | plateau::dataset::PredefinedCityModelPackage::UrbanPlanningDecision | plateau::dataset::PredefinedCityModelPackage::Vegetation;
    citygml::CityObject::CityObjectsType EnableCityObjects = citygml::CityObject::CityObjectsType::COT_All;

    APLATEAUInstancedCityModel* SelectingActor;

public:
    void Construct(const FArguments& InArgs, const TSharedRef<class FPLATEAUEditorStyle>& InStyle);

private:
    void SetAllFlags(const bool InBool);
    void SetAllBuildingSettingFlag(const bool InBool);
    void SetAllReliefSettingFlag(const bool InBool);
    void SetAllVegetationSettingFlag(const bool InBool);

    void SetMaxBuildingLOD(const int InLOD);
    void SetMinBuildingLOD(const int InLOD);
    int TargetMaxLOD = 999;
    int TargetMinLOD = 999;
    bool bActorSelected = false;

    void CheckTargetLODs();

    TSharedPtr<SBox> ConstructGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage, const FText PanelTitle);
    void SetEnableStatFromGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage, const bool InBool);
    bool GetEnableStatFromGenericPanel(const plateau::dataset::PredefinedCityModelPackage TargetPackage);
    TSharedPtr<SBox> ConstructGenericCheckBox(const plateau::dataset::PredefinedCityModelPackage TargetPackage,
        const citygml::CityObject::CityObjectsType TargetObjectType, const FText CheckBoxTitle);

    TSharedPtr<SVerticalBox> ConstructHeader();
    TSharedPtr<SBox> ConstructBuilldingPanel();
    TSharedPtr<SBox> ConstructReliefPanel();
    TSharedPtr<SBox> ConstructVegetationPanel();

    TSharedPtr<SVerticalBox> ConstructBuildingDetailsPanel();
    TSharedPtr<SVerticalBox> ConstructReliefDetailsPanel();
    TSharedPtr<SVerticalBox> ConstructVegetationDetailsPanel();
    TSharedPtr<SVerticalBox> ConstructSelectingActorPanel();

    TSharedPtr<SVerticalBox> ConstructGenericLODWidget(plateau::dataset::PredefinedCityModelPackage TargetPackage);
    FText GetLODText(plateau::dataset::PredefinedCityModelPackage TargetPackage);
    TSharedPtr<SVerticalBox> ConstructBuildingLODWidget();
    
    TOptional<int32> GetTargetMaxLOD() const { return TargetMaxLOD; };
    TOptional<int32> GetTargetMinLOD() const { return TargetMinLOD; };

};
