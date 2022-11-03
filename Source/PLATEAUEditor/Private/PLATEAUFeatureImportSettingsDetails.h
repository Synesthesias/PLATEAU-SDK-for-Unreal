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

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32;
}

class FPLATEAUFeatureSettingsRow {
public:
    FPLATEAUFeatureSettingsRow(const plateau::udx::PredefinedCityModelPackage InPackage)
        : Package(InPackage) {}
    
    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty) const;

private:
    plateau::udx::PredefinedCityModelPackage Package;
};

class FPLATEAUFeatureSettingsDetails : public IDetailCustomization {
public:
    FPLATEAUFeatureSettingsDetails(const TArray<plateau::udx::PredefinedCityModelPackage>& InPackages);

    static TSharedRef<IDetailCustomization> MakeInstance(const TArray<plateau::udx::PredefinedCityModelPackage> Packages) {
        return MakeShareable(new FPLATEAUFeatureSettingsDetails(Packages));
    }

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    TArray<plateau::udx::PredefinedCityModelPackage> Packages;
    TMap<plateau::udx::PredefinedCityModelPackage, FPLATEAUFeatureSettingsRow> FeatureSettingsRowMap;
};
