#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "PLATEAUCityMap.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"

class FFeaturePlacementRow {
public:
    FFeaturePlacementRow(const ECityModelPackage Package)
        : Package(Package) {}

    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty);

private:
    ECityModelPackage Package;

    // プロパティ
    TSharedPtr<IPropertyHandle> FeaturePlacementModeProperty;
    TSharedPtr<IPropertyHandle> FeatureLODProperty;

    TMap<EFeaturePlacementMode, FText> FeaturePlacementTexts() const {
        TMap<EFeaturePlacementMode, FText> Items;
        Items.Add(EFeaturePlacementMode::DontPlace, LOCTEXT("DontPlace", "配置無し"));
        Items.Add(EFeaturePlacementMode::PlaceTargetLOD, LOCTEXT("PlaceTargetLOD", "選択LODを配置、無ければ配置しない"));
        Items.Add(EFeaturePlacementMode::PlaceTargetLODOrLower, LOCTEXT("PlaceTargetLODOrLower", "選択LODを配置、無ければ最大LODを配置する"));
        return Items;
    }

private:
    TSharedRef<SWidget> OnGetFeaturePlacementModeComboContent() const;
    void CommitPlacementMode(EFeaturePlacementMode NewMode) const;
    FText GetComboButtonText() const;
};

#undef LOCTEXT_NAMESPACE
