// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "SPLATEAUFeatureImportSettingsView.h"

#include "SPLATEAUExtentEditButton.h"
#include <plateau/dataset/city_model_package.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>

#include "PLATEAUEditor.h"
#include "PLATEAUFeatureImportSettingsDetails.h"
#include "PLATEAUImportSettings.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "SlateOptMacros.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

using namespace plateau::dataset;

SPLATEAUFeatureImportSettingsView::SPLATEAUFeatureImportSettingsView() {}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SPLATEAUFeatureImportSettingsView::Construct(const FArguments& InArgs) {
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bAllowSearch = false;

    ImportSettingsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);

    ChildSlot
        [SNew(SVerticalBox) +
        SVerticalBox::Slot()
        // 動的更新のためのダミーWidget
        // TODO: 別の方法で動的更新可能か?
        [SNew(SBox)
        .Visibility_Lambda(
            [this, Extent = InArgs._Extent,
            InDatasetAccessor = InArgs._DatasetAccessor]() {
        if (ExtentCache == Extent.Get() && DatasetAccessor == InDatasetAccessor.Get())
            return EVisibility::Collapsed;

        ExtentCache = Extent.Get();
        DatasetAccessor = InDatasetAccessor.Get();

        auto PackageMask = PredefinedCityModelPackage::None;

        if (Extent.Get() != FPLATEAUExtent()) {
            const auto FilteredDatasetAccessor =
                DatasetAccessor->filter(Extent.Get().GetNativeData());

            PackageMask = FilteredDatasetAccessor->getPackages();
        }

        ImportSettingsView->UnregisterInstancedCustomPropertyLayout(UPLATEAUImportSettings::StaticClass());
        ImportSettingsView->RegisterInstancedCustomPropertyLayout(
            UPLATEAUImportSettings::StaticClass(),
            FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUFeatureSettingsDetails::MakeInstance, PackageMask));
        ImportSettingsView->SetObject(GetMutableDefault<UPLATEAUImportSettings>());
        ImportSettingsView->ForceRefresh();

        return EVisibility::Collapsed;
    })
        ] +
        SVerticalBox::Slot()
        .AutoHeight()
        [ImportSettingsView.ToSharedRef()]

        ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
