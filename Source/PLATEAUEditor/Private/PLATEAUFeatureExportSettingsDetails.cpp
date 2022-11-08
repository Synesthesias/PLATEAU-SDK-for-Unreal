// Copyright Epic Games, Inc. All Rights Reserved.

#include "PLATEAUFeatureExportSettingsDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/SBoxPanel.h"
#include "EditorFontGlyphs.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"

#include "PLATEAUExportSettings.h"

#define LOCTEXT_NAMESPACE "PLATEAUFeatureExportSettings"

namespace {
    TMap<EMeshTransformType, FText> GetCoordinateText() {
        TMap<EMeshTransformType, FText> Items;
        Items.Add(EMeshTransformType::Local, LOCTEXT("Local", "ローカル座標"));
        Items.Add(EMeshTransformType::PlaneRect, LOCTEXT("PlaneRect", "平面直角座標"));
        return Items;
    }
}

FPLATEAUExportFeatureSettingsRow::FPLATEAUExportFeatureSettingsRow() {
    //必要であれば
}

void FPLATEAUExportFeatureSettingsRow::AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeatureSettingsProperty) {
    auto ExportTextureProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureExportSettings, bExportTexture));
    auto ExportCoordinateProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureExportSettings, ExportCoordinate));
    auto ExportHiddenModelProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureExportSettings, bExportHiddenModel));

    // テクスチャを出力する
    Category.AddCustomRow(FText::FromString(TEXT("Export Texture")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Export Texture", "テクスチャを出力する"))]
        .ValueContent()[ExportTextureProperty->CreatePropertyValueWidget()];

    // 座標系
    Category.AddCustomRow(FText::FromString(TEXT("Coordinate")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Coordinate", "座標設定"))]
        .ValueContent()
        [SNew(SComboButton)
        .OnGetMenuContent_Lambda(
            [ExportCoordinateProperty]() {
                FMenuBuilder MenuBuilder(true, nullptr);
                const auto Items = GetCoordinateText();
                for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                    FText ItemText = ItemIter->Value;
                    auto ExportCoordinate = ItemIter->Key;
                    FUIAction ItemAction(FExecuteAction::CreateLambda(
                        [ExportCoordinateProperty, ExportCoordinate]() {
                            ExportCoordinateProperty->SetValue(static_cast<uint8>(ExportCoordinate));
                        }));
                    MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                }
                return MenuBuilder.MakeWidget();
            })
        .ContentPadding(0.0f)
                .VAlign(VAlign_Center)
                .ButtonContent()
                [SNew(STextBlock).Text_Lambda(
                    [ExportCoordinateProperty]() {
                        // TODO
                        const auto Texts = GetCoordinateText();

                        if (!ExportCoordinateProperty.IsValid())
                            return Texts[EMeshTransformType::Local];

                        uint8 Out;
                        ExportCoordinateProperty->GetValue(Out);
                        return Texts[static_cast<EMeshTransformType>(Out)];
                    })]];

    // 非表示モデルを出力する
    Category.AddCustomRow(FText::FromString(TEXT("Export Hidden Model")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Export Hidden Model", "非表示モデルを出力する"))]
        .ValueContent()[ExportHiddenModelProperty->CreatePropertyValueWidget()];
}

void FPLATEAUFeatureExportSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    FName CategoryName = TEXT("Option");
    FText LocalizedCategoryName = LOCTEXT("Option", "オプション");
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(CategoryName, LocalizedCategoryName);
    const auto FeatureSettingsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UPLATEAUExportSettings, ExportSetting));
    DetailBuilder.HideProperty(FeatureSettingsProperty);
    if (!bSettingReady) {
        FPLATEAUExportFeatureSettingsRow Setting;
        Setting.AddToCategory(Category, FeatureSettingsProperty);
    }
}

#undef LOCTEXT_NAMESPACE
