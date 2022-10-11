// Fill out your copyright notice in the Description page of Project Settings.

#include "PLATEAUCityModelLoaderDetails.h"

#include "SlateOptMacros.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"

#include "PLATEAUEditor.h"
#include "PLATEAUCityModelLoader.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"


#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"

TSharedRef<IDetailCustomization> FPLATEAUCityModelLoaderDetails::MakeInstance() {
    return MakeShareable(new FPLATEAUCityModelLoaderDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FPLATEAUCityModelLoaderDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    auto& CityModelCategory = DetailBuilder.EditCategory("CityModel", LOCTEXT("CityModel", "都市モデル"));
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    TWeakObjectPtr<APLATEAUCityModelLoader> CityModelLoader = Cast<APLATEAUCityModelLoader>(ObjectsBeingCustomized[0]);

    const auto CityModelPlacementSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityModelLoader, CityModelPlacementSettings));

    CityModelCategory.AddCustomRow(FText::FromString("EditExtent"))
        .WholeRowContent()
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [CityModelLoader]() {
                IPLATEAUEditorModule::Get().GetExtentEditor()->RegisterLoaderActor(CityModelLoader);

                IPLATEAUEditorModule::Get().GetExtentEditor()->SetSourcePath(CityModelLoader->Source);

                // TODO: ExtentEditorに委譲
                // ビューポートの操作性向上のため100分の1スケールで設定
                const plateau::geometry::GeoReference RawGeoReference({}, 1, plateau::geometry::CoordinateSystem::NWU, 9);
                IPLATEAUEditorModule::Get().GetExtentEditor()->SetGeoReference(RawGeoReference);

                const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
                GlobalTabManager->TryInvokeTab(FPLATEAUExtentEditor::TabId);

                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Edit Extent Button", "範囲選択"))
                ]
        ];

    CityModelCategory.AddCustomRow(FText::FromString("LoadCityModel"))
        .WholeRowContent()
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [CityModelLoader]() {
                CityModelLoader->Load();
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("CityModel Load Button", "読み込み"))
                ]
        ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
