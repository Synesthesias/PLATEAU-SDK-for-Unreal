#include "SPLATEAUFeatureImportSettingsView.h"

#include "SPLATEAUExtentEditButton.h"
#include <plateau/udx/city_model_package.h>
#include <plateau/udx/udx_file_collection.h>

#include "PLATEAUEditor.h"
#include "PLATEAUFeatureImportSettingsDetails.h"
#include "PLATEAUImportSettings.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "SlateOptMacros.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"

using namespace plateau::udx;

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
            SourcePath = InArgs._SourcePath]() {
        if (ExtentCache == Extent.Get() && SourcePathCache == SourcePath.Get())
            return EVisibility::Collapsed;

        ExtentCache = Extent.Get();
        SourcePathCache = SourcePath.Get();

        auto PackageMask = PredefinedCityModelPackage::None;

        if (Extent.Get() != FPLATEAUExtent()) {
            // TODO: 多重呼び出し解消
            const auto FileCollection =
                plateau::udx::UdxFileCollection::find(TCHAR_TO_UTF8(*SourcePath.Get()))
                ->filter(Extent.Get().GetNativeData());

            PackageMask = FileCollection->getPackages();
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
