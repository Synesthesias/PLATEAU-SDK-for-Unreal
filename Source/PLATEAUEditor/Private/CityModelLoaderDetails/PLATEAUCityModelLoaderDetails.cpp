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

    CityModelCategory.AddCustomRow(FText::FromString("LoadCityModel"))
        .WholeRowContent()
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [CityModelLoader]() {
                // TODO: SDK画面を開く
                return FReply::Handled();
            })
        .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Open SDK Window", "SDK画面を開く"))
                ]
        ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
