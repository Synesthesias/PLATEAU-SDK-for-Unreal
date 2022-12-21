#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"

class IDetailLayoutBuilder;
class FUICommandInfo;
class SWidget;
class SEditableTextBox;
class SSequenceRecorder;

namespace plateau::dataset {
    enum class PredefinedCityModelPackage : uint32;
}

class FPLATEAUFeatureSettingsRow {
public:
    FPLATEAUFeatureSettingsRow(const plateau::dataset::PredefinedCityModelPackage InPackage)
        : Package(InPackage) {}
    
    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty) const;

private:
    plateau::dataset::PredefinedCityModelPackage Package;
};

class FPLATEAUFeatureSettingsDetails : public IDetailCustomization {
public:
    FPLATEAUFeatureSettingsDetails(const plateau::dataset::PredefinedCityModelPackage InPackageMask);

    static TSharedRef<IDetailCustomization> MakeInstance(const plateau::dataset::PredefinedCityModelPackage InPackageMask) {
        return MakeShareable(new FPLATEAUFeatureSettingsDetails(InPackageMask));
    }

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    plateau::dataset::PredefinedCityModelPackage PackageMask;
    TMap<plateau::dataset::PredefinedCityModelPackage, FPLATEAUFeatureSettingsRow> FeatureSettingsRowMap;
};
