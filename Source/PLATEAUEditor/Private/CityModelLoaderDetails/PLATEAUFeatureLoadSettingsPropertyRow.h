#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "PLATEAUCityModelLoader.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityModelLoaderDetails"

class FPLATEAUFeatureLoadSettingsPropertyRow {
public:
    FPLATEAUFeatureLoadSettingsPropertyRow(/*const ECityModelPackage Package*/)
         /* : Package(Package)*/ {}

    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty);

private:
    //ECityModelPackage Package;

    // プロパティ
    TSharedPtr<IPropertyHandle> FeaturePlacementModeProperty;
    TSharedPtr<IPropertyHandle> FeatureLODProperty;

private:
    //TSharedRef<SWidget> OnGetFeaturePlacementModeComboContent() const;
    //void CommitPlacementMode(EFeaturePlacementMode NewMode) const;
    //FText GetComboButtonText() const;
};

#undef LOCTEXT_NAMESPACE
