#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

class FPLATEAUServerConnectionSettingsDetails : public IDetailCustomization {
public:
    static TSharedRef<IDetailCustomization> MakeInstance() {
        return MakeShareable(new FPLATEAUServerConnectionSettingsDetails());
    }

    /** IDetailCustomization interface */
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
    bool bSettingReady = false;
};
