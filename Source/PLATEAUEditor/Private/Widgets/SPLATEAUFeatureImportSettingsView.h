#pragma once
#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "Widgets/SCompoundWidget.h"

namespace plateau::udx {
    enum class PredefinedCityModelPackage : uint32;
    class UdxFileCollection;
}

/**
 *
 */
class SPLATEAUFeatureImportSettingsView : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUFeatureImportSettingsView)
        : _SourcePath("")
        , _Extent() {}

        SLATE_ATTRIBUTE(FString, SourcePath)
        SLATE_ATTRIBUTE(FPLATEAUExtent, Extent)

        SLATE_END_ARGS()

public:
    SPLATEAUFeatureImportSettingsView();

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    
private:
    FPLATEAUExtent ExtentCache;
    FString SourcePathCache;

    TSharedPtr<IDetailsView> ImportSettingsView = nullptr;
};
