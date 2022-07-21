#include "FeaturePlacementSettingsPropertyRow.h"

#include "PLATEAUCityMapDetails.h"

#include "SlateOptMacros.h"
#include "PropertyHandle.h"
#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSlider.h"
#include "IDetailPropertyRow.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PLATEAUCityMap.h"

#include "citygml/citygml.h"
#include "citygml/citymodel.h"

#define LOCTEXT_NAMESPACE "PLATEAUCityMapDetails"


namespace {
    int GetIntProperty(TSharedPtr<IPropertyHandle> Property) {
        int Value;
        Property->GetValue(Value);
        return Value;
    }
}



TSharedRef<IDetailCustomization> FPLATEAUCityMapDetails::MakeInstance() {
    return MakeShareable(new FPLATEAUCityMapDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FFeaturePlacementRow::AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeaturePlacementSettingsProperty) {
    FeaturePlacementModeProperty = FeaturePlacementSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFeaturePlacementSettings, FeaturePlacementMode));
    FeatureLODProperty = FeaturePlacementSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFeaturePlacementSettings, TargetLOD));

    auto& FeaturePlacementSettingsRow = Category.AddProperty(FeaturePlacementSettingsProperty);
    FeaturePlacementSettingsRow.DisplayName(FCityModelPlacementSettings::GetDisplayName(Package));
    FeaturePlacementSettingsRow.CustomWidget()
        .NameContent()[FeaturePlacementSettingsProperty->CreatePropertyNameWidget()]
        .ValueContent()
        .MaxDesiredWidth(200)
        [SNew(SVerticalBox)
        + SVerticalBox::Slot()
        [SNew(SComboButton)
        .OnGetMenuContent_Raw(this, &FFeaturePlacementRow::OnGetFeaturePlacementModeComboContent)
        .ContentPadding(0.0f)
        .ButtonStyle(FEditorStyle::Get(), "ToggleButton")
        .ForegroundColor(FSlateColor::UseForeground())
        .VAlign(VAlign_Center)
        .ButtonContent()
        [SNew(STextBlock).Text_Raw(this, &FFeaturePlacementRow::GetComboButtonText)]]
    + SVerticalBox::Slot()
        [SNew(SHorizontalBox)
        .Visibility_Lambda(
            [this]() {
                uint8 out;
                FeaturePlacementModeProperty->GetValue(out);
                return static_cast<EFeaturePlacementMode>(out) != EFeaturePlacementMode::DontPlace ? EVisibility::Visible : EVisibility::Hidden;
            }
        )
        + SHorizontalBox::Slot()
                [SNew(STextBlock).Text_Lambda(
                    [this]() {
                        int LOD = GetIntProperty(FeatureLODProperty);
                        return FText::Format(LOCTEXT("LODFormat", "LOD: {0}"), FText::AsNumber(LOD));
                    })
                ]
            + SHorizontalBox::Slot()
                [SNew(SSlider)
                .MaxValue(3)
                .MinValue(0)
                .StepSize(1)
                .MouseUsesStep(true)
                .Value(GetIntProperty(FeatureLODProperty))
                .OnValueChanged_Lambda(
                    [this](int Value) {
                        FeatureLODProperty->SetValue(Value);
                    })
                ]]];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FText FFeaturePlacementRow::GetComboButtonText() const {
    uint8 out;
    FeaturePlacementModeProperty->GetValue(out);
    return FeaturePlacementTexts()[static_cast<EFeaturePlacementMode>(out)];
}

TSharedRef<SWidget> FFeaturePlacementRow::OnGetFeaturePlacementModeComboContent() const {
    FMenuBuilder MenuBuilder(true, nullptr);
    auto Items = FeaturePlacementTexts();
    for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
        FText ItemText = ItemIter->Value;
        EFeaturePlacementMode Mode = ItemIter->Key;
        FUIAction ItemAction(FExecuteAction::CreateLambda([this, Mode]() {
            CommitPlacementMode(Mode);
            }));
        MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
    }

    return MenuBuilder.MakeWidget();
}

void FFeaturePlacementRow::CommitPlacementMode(EFeaturePlacementMode NewMode) const {
    FeaturePlacementModeProperty->SetValue(static_cast<uint8>(NewMode));
}


#undef LOCTEXT_NAMESPACE
