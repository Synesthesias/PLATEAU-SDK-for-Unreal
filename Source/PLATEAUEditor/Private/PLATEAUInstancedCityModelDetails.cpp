// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "PLATEAUInstancedCityModelDetails.h"
#include "PLATEAUInstancedCityModel.h"

#include "DetailLayoutBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "PLATEAUInstancedCityModelDetails"

TSharedRef<IDetailCustomization> FPLATEAUInstancedCityModelDetails::MakeInstance() {
    return MakeShareable(new FPLATEAUInstancedCityModelDetails);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void FPLATEAUInstancedCityModelDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {
    auto& CityModelCategory = DetailBuilder.EditCategory("Origin Information", LOCTEXT("Origin Information", "原点情報"));
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
    TWeakObjectPtr<APLATEAUInstancedCityModel> CityModel = Cast<APLATEAUInstancedCityModel>(ObjectsBeingCustomized[0]);
    
    CityModelCategory.AddCustomRow(FText::FromString("Lat"))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Lat", "緯度"))]
        .ValueContent()
        [SNew(STextBlock)
        .Text_Lambda(
            [CityModel]() {
                const auto Latitude = CityModel->GeoReference.GetData().unproject(TVec3d(0, 0, 0)).latitude;
                return FText::FromString(FString::SanitizeFloat(Latitude));
            })
        ];
    CityModelCategory.AddCustomRow(FText::FromString("Lon"))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Lon", "経度"))]
        .ValueContent()
        [SNew(STextBlock)
        .Text_Lambda(
            [CityModel]() {
                const auto Longitude = CityModel->GeoReference.GetData().unproject(TVec3d(0, 0, 0)).longitude;
                return FText::FromString(FString::SanitizeFloat(Longitude));
            })
        ];
    CityModelCategory.AddCustomRow(FText::FromString("Height"))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Hegight", "高さ"))]
        .ValueContent()
        [SNew(STextBlock)
        .Text_Lambda(
            [CityModel]() {
                const auto Height= CityModel->GeoReference.GetData().unproject(TVec3d(0, 0, 0)).height;
                return FText::FromString(FString::SanitizeFloat(Height));
            })
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
