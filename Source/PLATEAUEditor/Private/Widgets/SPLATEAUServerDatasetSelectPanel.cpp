#include "SPLATEAUServerDatasetSelectPanel.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#include <plateau/dataset/city_model_package.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>
#include <plateau/network/client.h>

#define LOCTEXT_NAMESPACE "SPLATEAUServerDatasetSelectPanel"

namespace {
    TMap<int, FText> GetPrefectureTexts() {
        TMap<int, FText> Items;
        Items.Add(1, LOCTEXT("Hokkaido", "北海道"));
        Items.Add(2, LOCTEXT("Aomori", "青森県"));
        Items.Add(3, LOCTEXT("Iwate", "岩手県"));
        Items.Add(4, LOCTEXT("Miyagi", "宮城県"));
        Items.Add(5, LOCTEXT("Akita", "秋田県"));
        Items.Add(6, LOCTEXT("Yamagata", "山形県"));
        Items.Add(7, LOCTEXT("Fukushima", "福島県"));
        Items.Add(8, LOCTEXT("Ibaraki", "茨城県"));
        Items.Add(9, LOCTEXT("Tochigi", "栃木県"));
        Items.Add(10, LOCTEXT("Gunma", "群馬県"));
        Items.Add(11, LOCTEXT("Saitama", "埼玉県"));
        Items.Add(12, LOCTEXT("Chiba", "千葉県"));
        Items.Add(13, LOCTEXT("Tokyo", "東京都"));
        Items.Add(14, LOCTEXT("Kanagawa", "神奈川県"));
        Items.Add(15, LOCTEXT("Nigata", "新潟県"));
        Items.Add(16, LOCTEXT("Toyama", "富山県"));
        Items.Add(17, LOCTEXT("Ishikawa", "石川県"));
        Items.Add(18, LOCTEXT("Fukui", "福井県"));
        Items.Add(19, LOCTEXT("Yamanashi", "山梨県"));
        Items.Add(20, LOCTEXT("Nagano", "長野県"));
        Items.Add(21, LOCTEXT("Gifu", "岐阜県"));
        Items.Add(22, LOCTEXT("Shizuoka", "静岡県"));
        Items.Add(23, LOCTEXT("Aichi", "愛知県"));
        Items.Add(24, LOCTEXT("Mie", "三重県"));
        Items.Add(25, LOCTEXT("Shiga", "滋賀県"));
        Items.Add(26, LOCTEXT("Kyoto", "京都府"));
        Items.Add(27, LOCTEXT("Osaka", "大阪府"));
        Items.Add(28, LOCTEXT("Hyogo", "兵庫県"));
        Items.Add(29, LOCTEXT("Nara", "奈良県"));
        Items.Add(30, LOCTEXT("Wakayama", "和歌山県"));
        Items.Add(31, LOCTEXT("Tottori", "鳥取県"));
        Items.Add(32, LOCTEXT("Shimane", "埼玉県"));
        Items.Add(33, LOCTEXT("Okayama", "岡山県"));
        Items.Add(34, LOCTEXT("Hiroshima", "広島県"));
        Items.Add(35, LOCTEXT("Yamaguchi", "山口県"));
        Items.Add(36, LOCTEXT("Tokushima", "徳島県"));
        Items.Add(37, LOCTEXT("Kagawa", "香川県"));
        Items.Add(38, LOCTEXT("Ehime", "愛媛県"));
        Items.Add(39, LOCTEXT("Kochi", "高知県"));
        Items.Add(40, LOCTEXT("Fukuoka", "福岡県"));
        Items.Add(41, LOCTEXT("Saga", "佐賀県"));
        Items.Add(42, LOCTEXT("Nagasaki", "長崎県"));
        Items.Add(43, LOCTEXT("Kumamoto", "熊本県"));
        Items.Add(44, LOCTEXT("Oita", "大分県"));
        Items.Add(45, LOCTEXT("Miyazaki", "宮崎県"));
        Items.Add(46, LOCTEXT("Kagoshima", "鹿児島県"));
        Items.Add(47, LOCTEXT("Okinawa", "沖縄県"));
        return Items;
    }

    TMap<int, FText> GetMunicipalityTexts() {
        TMap<int, FText> Items;
        Items.Add(1, LOCTEXT("Municipality01", "市町村1"));
        Items.Add(2, LOCTEXT("Municipality02", "市町村2"));
        Items.Add(3, LOCTEXT("Municipality03", "市町村3"));
        Items.Add(4, LOCTEXT("Municipality04", "市町村4"));
        Items.Add(5, LOCTEXT("Municipality05", "市町村5"));
        Items.Add(6, LOCTEXT("Municipality06", "市町村6"));
        Items.Add(7, LOCTEXT("Municipality07", "市町村7"));
        Items.Add(8, LOCTEXT("Municipality08", "市町村8"));
        Items.Add(9, LOCTEXT("Municipality09", "市町村9"));
        Items.Add(10, LOCTEXT("Municipality10", "市町村10"));
        return Items;
    }
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUServerDatasetSelectPanel::Construct(const FArguments& InArgs) {
    OwnerWindow = InArgs._OwnerWindow;

    auto ClientRef = plateau::network::Client();
    std::vector<plateau::network::DatasetMetadataGroup> DataSets;
    ClientRef.getMetadata(DataSets);
    TMap<int, FText> PrefectureTexts;
    TMap<int, FText> MunicipalityTexts;
    for (int i = 0; i < DataSets.size(); i++) {
        PrefectureTexts.Add(i + 1, FText::FromString(DataSets[i].title.c_str()));
    }

    ChildSlot[
        SNew(SVerticalBox)
            .Visibility_Lambda([this]() {
            return bIsVisible ? EVisibility::Visible : EVisibility::Collapsed;
            }) +
            SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 0, 0, 15))
                    [
                        SNew(SHorizontalBox) +
                        SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(0.1f) +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Right)
                    .Padding(FMargin(0, 0, 0, 5))
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Justification(ETextJustify::Right)
                    .Text(LOCTEXT("Select Prefecture", "都道府県"))
                    ] +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(0.1f) +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(1)
                    [
                        SNew(SComboButton)
                        .OnGetMenuContent_Lambda(
                            [&]() {
                                FMenuBuilder MenuBuilder(true, nullptr);
                                const auto Items = GetPrefectureTexts();
                                for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                                    const auto ItemText = ItemIter->Value;
                                    const auto ID = ItemIter->Key;
                                    FUIAction ItemAction(FExecuteAction::CreateLambda(
                                        [this, ID]() {
                                            PrefectureID = ID;

                                            //リセットするのが適切？
                                            MunicipalityID = 1;
                                        }));
                                    MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                                }
                                return MenuBuilder.MakeWidget();
                            })
                    .ContentPadding(0.0f)
                                .VAlign(VAlign_Center)
                                .ButtonContent()
                                [
                                    SNew(STextBlock).Text_Lambda(
                                        [this]() {
                                            return GetPrefectureTexts()[PrefectureID];
                                        })
                                .Justification(ETextJustify::Left)
                                ]
                    ]

                //市町村
                //TODO : サーバーから受け取ったデータの反映
                +SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 15)
                    .FillWidth(0.1f)]
                + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(0, 0, 0, 5))
                    [
                        SNew(SHorizontalBox) +
                        SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 15)
                    .FillWidth(0.1f) +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Right)
                    .Padding(FMargin(0, 0, 0, 5))
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("Select Municipality", "市町村"))
                    .Justification(ETextJustify::Right)
                    ] +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(0.1f) +
                    SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(1)
                    [
                        SNew(SComboButton)
                        .OnGetMenuContent_Lambda(
                            [&]() {
                                FMenuBuilder MenuBuilder(true, nullptr);
                                const auto Items = GetMunicipalityTexts();
                                for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                                    const auto ItemText = ItemIter->Value;
                                    const auto ID = ItemIter->Key;
                                    FUIAction ItemAction(FExecuteAction::CreateLambda(
                                        [this, ID]() {
                                            MunicipalityID = ID;
                                        }));
                                    MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                                }
                                return MenuBuilder.MakeWidget();
                            })
                    .ContentPadding(0.0f)
                                .VAlign(VAlign_Center)
                                .ButtonContent()
                                [
                                    SNew(STextBlock).Text_Lambda(
                                        [this]() {
                                            return GetMunicipalityTexts()[MunicipalityID];
                                        })
                                .Justification(ETextJustify::Left)
                                ]
                    ]
                + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 0, 5)
                    .FillWidth(0.1f)
                    ]
                + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(FMargin(32, 0, 32, 0))[
                        SNew(SBox)
                            .WidthOverride(5)
                            .HeightOverride(30)
                            [SNew(SButton)
                            .HAlign(HAlign_Center)
                            .VAlign(VAlign_Center)
                            .ForegroundColor(FColor::White)
                            .ButtonColorAndOpacity(FColor(132, 132, 132))
                            .OnClicked_Lambda([&]() {
                            return FReply::Handled();
                                })
                            .Content()
                                    [SNew(STextBlock)
                                    .Justification(ETextJustify::Center)
                                    .Text(LOCTEXT("Update", "更新"))
                                    ]
                            ]

                    ]
                    + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(32, 10, 0, 5))
                        [SNew(STextBlock)
                        .Justification(ETextJustify::Left)
                        .Text(LOCTEXT("Max LOD", "最大LOD"))
                        ]
                    + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(32, 0, 32, 0))
                        [//TODO : 最大LODの反映
                            SNew(SEditableTextBox)
                            .Justification(ETextJustify::Left)
                        .Text(LOCTEXT("3", "3"))
                        .IsReadOnly(true)
                        ]
                    + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(32, 15, 0, 5))
                        [
                            SNew(STextBlock)
                            .Justification(ETextJustify::Left)
                        .Text(LOCTEXT("Model Description", "説明"))
                        ]
                    + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(32, 0, 32, 0))
                        [
                            SNew(SBox)
                            .WidthOverride(180)
                        .HeightOverride(90)
                        [
                            SNew(SMultiLineEditableTextBox)
                            .Justification(ETextJustify::Left)
                        .IsReadOnly(true)
                        .AutoWrapText(true)
                        .Text(LOCTEXT("Description Here", "ここに説明文が入ります。"))
                        ]
                        ]
    ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
