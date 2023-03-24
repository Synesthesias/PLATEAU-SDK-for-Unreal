// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUServerConnectionSettingsDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"

#include "PLATEAUServerConnectionSettings.h"

#define LOCTEXT_NAMESPACE "PLATEAUFeatureExportSettings"

void FPLATEAUServerConnectionSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    FName CategoryName = TEXT("Connection Settings Override");
    FText LocalizedCategoryName = LOCTEXT("Connection Settings Override", "接続先上書き設定");
    IDetailCategoryBuilder& Category =
        DetailBuilder.EditCategory(CategoryName, LocalizedCategoryName);
    const auto UrlProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UPLATEAUServerConnectionSettings, Url));
    DetailBuilder.HideProperty(UrlProperty);
    Category.AddCustomRow(FText::FromString(TEXT("Url")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Url", "サーバーURL"))]
        .ValueContent()[UrlProperty->CreatePropertyValueWidget()];

    const auto TokenProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UPLATEAUServerConnectionSettings, Token));
    DetailBuilder.HideProperty(TokenProperty);
    Category.AddCustomRow(FText::FromString(TEXT("Url")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Token", "トークン"))]
        .ValueContent()[TokenProperty->CreatePropertyValueWidget()];
}

#undef LOCTEXT_NAMESPACE
