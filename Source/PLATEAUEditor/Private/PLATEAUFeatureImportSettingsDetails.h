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
    
    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty);

private:
    plateau::udx::PredefinedCityModelPackage Package;
};

class FPLATEAUFeatureSettingsDetails : public IDetailCustomization {
public:
    static TSharedRef<IDetailCustomization> MakeInstance() {
        return MakeShareable(new FPLATEAUFeatureSettingsDetails());
    }

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    TMap<plateau::udx::PredefinedCityModelPackage, FPLATEAUFeatureSettingsRow> FeatureSettingsRowMap;
};
