#pragma once
#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "Widgets/SCompoundWidget.h"

namespace plateau::dataset {
    class IDatasetAccessor;
}

/**
 *
 */
class SPLATEAUFeatureImportSettingsView : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPLATEAUFeatureImportSettingsView)
        : _DatasetAccessor(nullptr)
        , _Extent() {}

        SLATE_ATTRIBUTE(std::shared_ptr<plateau::dataset::IDatasetAccessor>, DatasetAccessor)
        SLATE_ATTRIBUTE(FPLATEAUExtent, Extent)

        SLATE_END_ARGS()

public:
    SPLATEAUFeatureImportSettingsView();

    /** Constructs this widget with InArgs */
    void Construct(const FArguments& InArgs);
    
private:
    FPLATEAUExtent ExtentCache;
    std::shared_ptr<plateau::dataset::IDatasetAccessor> DatasetAccessor;

    TSharedPtr<IDetailsView> ImportSettingsView = nullptr;
};
