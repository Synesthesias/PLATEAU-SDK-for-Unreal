// Copyright © 2023 Ministry of Land, Infrastructure and Transport
#include "PLATEAUCityObjectGroupDetails.h"
#include "PLATEAUCityObjectGroup.h"
#include "SlateOptMacros.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

namespace  {
    constexpr float TextBoxDesiredHeight = 250.0f;
}

#define LOCTEXT_NAMESPACE "PLATEAUCityObjectGroupDetails"

TSharedRef<IDetailCustomization> FPLATEAUCityObjectGroupDetails::MakeInstance() {
    return MakeShareable(new FPLATEAUCityObjectGroupDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FPLATEAUCityObjectGroupDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    auto& CityObjectGroupCategory = DetailBuilder.EditCategory("PLATEAU", FText::GetEmpty(), ECategoryPriority::Important);
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup = Cast<UPLATEAUCityObjectGroup>(ObjectsBeingCustomized[0]);

    CityObjectGroupCategory.AddCustomRow(FText::FromString("CityObjectGroup")).WholeRowContent()
    [
        SNew(SScrollBox).Orientation(Orient_Vertical)
        +SScrollBox::Slot()
        [
            SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight()
            [
                SNew(SBox).MaxDesiredHeight(TextBoxDesiredHeight).MinDesiredHeight(TextBoxDesiredHeight)
                [
                    SNew(SMultiLineEditableTextBox)
                    .Text_Lambda([CityObjectGroup] {
                        return FText::FromString(CityObjectGroup->SerializedCityObjects);
                    })
                    .IsReadOnly(true)
                ]
            ]
        ]
    ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
