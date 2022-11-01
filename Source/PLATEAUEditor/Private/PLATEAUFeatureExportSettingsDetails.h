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

class FPLATEAUExportFeatureSettingsRow {
public:
    FPLATEAUExportFeatureSettingsRow();

    void AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty);
};

class FPLATEAUFeatureExportSettingsDetails : public IDetailCustomization {
public:
    static TSharedRef<IDetailCustomization> MakeInstance() {
        return MakeShareable(new FPLATEAUFeatureExportSettingsDetails());
    }

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    bool bSettingReady = false;
};
