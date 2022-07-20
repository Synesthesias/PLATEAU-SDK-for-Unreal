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

    static FText GetDisplayName(ECityModelPackage Package);
private:
    ECityModelPackage Package;

    // プロパティ
    TSharedPtr<IPropertyHandle> FeaturePlacementModeProperty;
    TSharedPtr<IPropertyHandle> FeatureLODProperty;

    TMap<EFeaturePlacementMode, FText> FeaturePlacementTexts() const {
        TMap<EFeaturePlacementMode, FText> Items;
        Items.Add(EFeaturePlacementMode::DontPlace, LOCTEXT("DontPlace", "配置無し"));
        Items.Add(EFeaturePlacementMode::PlaceMaxLOD, LOCTEXT("PlaceMaxLOD", "最大LODを配置"));
        return Items;
    }

private:
    TSharedRef<SWidget> OnGetFeaturePlacementModeComboContent() const;
    void CommitPlacementMode(EFeaturePlacementMode NewMode) const;
    FText GetComboButtonText() const;
};

#undef LOCTEXT_NAMESPACE
