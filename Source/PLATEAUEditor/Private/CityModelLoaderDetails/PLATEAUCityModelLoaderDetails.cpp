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
    auto& CityModelCategory = DetailBuilder.EditCategory("PLATEAU");
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);
    TWeakObjectPtr<APLATEAUCityModelLoader> CityModelLoader = Cast<APLATEAUCityModelLoader>(ObjectsBeingCustomized[0]);

    const auto StatusProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(APLATEAUCityModelLoader, Status));
    DetailBuilder.HideProperty(StatusProperty);


    CityModelCategory.AddCustomRow(FText::FromString("LoadCityModel"))
        .WholeRowContent()
        [SNew(SVerticalBox) +
        SVerticalBox::Slot()
        .AutoHeight()
        [SNew(STextBlock)
        .Text_Lambda(
            [CityModelLoader]() {
                const auto HasLoadCompleted = CityModelLoader->Status.LoadedGmlCount == CityModelLoader->Status.TotalGmlCount;
                return HasLoadCompleted
                    ? LOCTEXT("Load Completed", "都市モデルの読み込みが完了しました。")
                    : LOCTEXT("Load In Progress", "都市モデルを読み込み中...");
            })
        ] +
        SVerticalBox::Slot()
                .AutoHeight()
                [SNew(SSpinBox<float>)
                .MinSliderValue(0.0f)
                .MaxSliderValue(100.0f)
                .Value_Lambda(
                    [CityModelLoader]() {
                        const auto Loaded = CityModelLoader->Status.LoadedGmlCount;
                        const auto Total = CityModelLoader->Status.TotalGmlCount;
                        if (Total == 0)
                            return 0.0f;
                        return static_cast<float>(Loaded * 100 / Total);
                    })
                ] +
                SVerticalBox::Slot()
                        .Padding(FMargin(0, 0, 0, 10))
                        .AutoHeight()
                        [SNew(STextBlock)
                        .Text_Lambda(
                            [CityModelLoader] {
                                FString Message;
                                for (const auto& GmlName : CityModelLoader->Status.LoadingGmls) {
                                    Message += FString::Format(
                                        TEXT("{0} を読み込み中...\n"),
                                        { *GmlName });
                                }
                                return FText::FromString(Message);
                            })
                        ]

        ];

    //CityModelCategory.AddCustomRow(FText::FromString("LoadCityModel"))
    //    .WholeRowContent()
    //    [
    //        SNew(SButton)
    //        .VAlign(VAlign_Center)
    //    .ForegroundColor(FColor::White)
    //    .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
    //    .OnClicked_Lambda(
    //        [CityModelLoader]() {
    //            // TODO: SDK画面を開く
    //            return FReply::Handled();
    //        })
    //    .Content()
    //            [
    //                SNew(STextBlock)
    //                .Justification(ETextJustify::Center)
    //            .Margin(FMargin(0, 5, 0, 5))
    //            .Text(LOCTEXT("Open SDK Window", "SDK画面を開く"))
    //            ]
    //    ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
