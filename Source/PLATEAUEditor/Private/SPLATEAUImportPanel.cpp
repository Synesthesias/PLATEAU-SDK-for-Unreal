// Fill out your copyright notice in the Description page of Project Settings.


#include "SPLATEAUImportPanel.h"

#include <plateau/udx/city_model_package.h>
#include <plateau/udx/udx_file_collection.h>

#include "PLATEAUCityModelLoader.h"
#include "PLATEAUImportSettings.h"
#include "Widgets/SPLATEAUExtentEditButton.h"
#include "Widgets/SPLATEAUFeatureImportSettingsView.h"

#include "AssetSelection.h"
#include "DesktopPlatformModule.h"
#include "PLATEAUEditor.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUFeatureImportSettingsDetails.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "SlateOptMacros.h"
#include "StatusBarSubsystem.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"

#define LOCTEXT_NAMESPACE "SPLATEAUImportPanel"

using namespace plateau::udx;

namespace {
    TMap<int, FText> GetZoneIDTexts() {
        TMap<int, FText> Items;
        Items.Add(1, LOCTEXT("Zone01", "01: 長崎, 鹿児島(南西部)"));
        Items.Add(2, LOCTEXT("Zone02", "02: 福岡, 佐賀, 熊本, 大分, 宮崎, 鹿児島(北東部)"));
        Items.Add(3, LOCTEXT("Zone03", "03: 山口, 島根, 広島"));
        Items.Add(4, LOCTEXT("Zone04", "04: 香川, 愛媛, 徳島, 高知"));
        Items.Add(5, LOCTEXT("Zone05", "05: 兵庫, 鳥取, 岡山"));
        Items.Add(6, LOCTEXT("Zone06", "06: 京都, 大阪, 福井, 滋賀, 三重, 奈良, 和歌山"));
        Items.Add(7, LOCTEXT("Zone07", "07: 石川, 富山, 岐阜, 愛知"));
        Items.Add(8, LOCTEXT("Zone08", "08: 新潟, 長野, 山梨, 静岡"));
        Items.Add(9, LOCTEXT("Zone09", "09: 東京(本州), 福島, 栃木, 茨城, 埼玉, 千葉, 群馬, 神奈川"));
        Items.Add(10, LOCTEXT("Zone10", "10: 青森, 秋田, 山形, 岩手, 宮城"));
        Items.Add(11, LOCTEXT("Zone11", "11: 北海道(西部)"));
        Items.Add(12, LOCTEXT("Zone12", "12: 北海道(中央部)"));
        Items.Add(13, LOCTEXT("Zone13", "13: 北海道(東部)"));
        Items.Add(14, LOCTEXT("Zone14", "14: 諸島(東京南部)"));
        Items.Add(15, LOCTEXT("Zone15", "15: 沖縄"));
        Items.Add(16, LOCTEXT("Zone16", "16: 諸島(沖縄西部)"));
        Items.Add(17, LOCTEXT("Zone17", "17: 諸島(沖縄東部)"));
        Items.Add(18, LOCTEXT("Zone18", "18: 小笠原諸島"));
        Items.Add(19, LOCTEXT("Zone19", "19: 南鳥島"));
        return Items;
    }

    static FText GetDisplayName(PredefinedCityModelPackage Package) {
        switch (Package) {
        case PredefinedCityModelPackage::Building: return LOCTEXT("Building", "建築物");
        case PredefinedCityModelPackage::Road: return LOCTEXT("Transportation", "道路");
        case PredefinedCityModelPackage::UrbanPlanningDecision: return LOCTEXT("UrbanPlanningDecision", "都市計画決定情報");
        case PredefinedCityModelPackage::LandUse: return LOCTEXT("LandUse", "土地利用");
        case PredefinedCityModelPackage::Relief: return LOCTEXT("Relief", "起伏");
        case PredefinedCityModelPackage::CityFurniture: return LOCTEXT("UrbanFacility", "都市設備");
        case PredefinedCityModelPackage::Vegetation: return LOCTEXT("Vegetation", "植生");
        case PredefinedCityModelPackage::DisasterRisk: return LOCTEXT("DisasterRisk", "災害リスク");
        default: return LOCTEXT("Others", "その他");
        }
    }

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
void SPLATEAUImportPanel::Construct(const FArguments& InArgs, const TSharedRef<FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;

    TWeakPtr<SVerticalBox> VerticalBox;
    TWeakPtr<SVerticalBox> ModelDataPlacementVerticalBox;

    ChildSlot
        [SAssignNew(VerticalBox, SVerticalBox)
        // モデルデータのインポート(ヘッダー)
        + SVerticalBox::Slot()
        .Padding(FMargin(0, 20.5, 0, 5))
        .AutoHeight()
        [SNew(SHeader)
        .HAlign(HAlign_Center)
        .Content()
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading1")
        .Text(LOCTEXT("Import ModelData", "モデルデータのインポートを行います。"))]]

    // 都市の追加(ヘッダー)
    + SVerticalBox::Slot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        [SNew(SHeader)
        .Content()
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(SImage)
        .Image(Style->GetBrush("PLATEAUEditor.BuildingIconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("Add City", "都市の追加"))]]]

    //インポート先
    + SVerticalBox::Slot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Import From", "インポート先"))
        ] +
        SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        ]

    //インポート先選択ボタン
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [CreateFileSourceSelectButton()]

    // 都市の追加(ローカルファイル)
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [CreateSourcePathSelectPanel()]

    // データセット選択(サーバーからのファイル)
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [CreateServerPrefectureSelectPanel()]

    // モデルデータの配置
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [SAssignNew(ModelDataPlacementVerticalBox, SVerticalBox)
        .Visibility_Lambda(
            [this]() {
                auto IsValidDatasetSet = false;
                if (FileCollection != nullptr)
                    IsValidDatasetSet = FileCollection->getPackages() != PredefinedCityModelPackage::None;
                return IsValidDatasetSet
                    ? EVisibility::Visible
                    : EVisibility::Collapsed;
            })
        ]

        ];

    // モデルデータの配置(ヘッダー)
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .Padding(FMargin(0, 15, 0, 5))
        [SNew(SHeader)
        .HAlign(HAlign_Center)
        .Content()
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading1")
        .Text(LOCTEXT("Place ModelData", "モデルデータの配置を行います。"))]];

    // 基準座標系の選択(ヘッダー)
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        [SNew(SHeader)
        .Content()
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(SImage)
        .Image(Style->GetBrush("PLATEAUEditor.Section1IconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("Select ReferenceCoordinate", "基準座標系の選択"))]]];

    // 基準座標系の選択
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .Padding(19, 0, 0, 0)
        [SNew(STextBlock)
        .Text(LOCTEXT("ReferenceCoordinate", "基準座標系"))] +
        SHorizontalBox::Slot()
        .Padding(0, 0, 19, 0)
        [SNew(SComboButton)
        .OnGetMenuContent_Lambda(
            [this]() {
                FMenuBuilder MenuBuilder(true, nullptr);
                const auto Items = GetZoneIDTexts();
                for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                    const auto ItemText = ItemIter->Value;
                    const auto ID = ItemIter->Key;
                    FUIAction ItemAction(FExecuteAction::CreateLambda(
                        [this, ID]() {
                            ZoneID = ID;
                        }));
                    MenuBuilder.AddMenuEntry(ItemText, TAttribute<FText>(), FSlateIcon(), ItemAction);
                }
                return MenuBuilder.MakeWidget();
            })
        .ContentPadding(0.0f)
                .VAlign(VAlign_Center)
                .ButtonContent()
                [SNew(STextBlock).Text_Lambda(
                    [this]() {
                        // TODO: キャッシュ化
                        return GetZoneIDTexts()[ZoneID];
                    })]]];

    // マップ範囲選択(ヘッダー)
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        .VAlign(VAlign_Center)
        [SNew(SHeader)
        .Content()
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(SImage)
        .Image(Style->GetBrush("PLATEAUEditor.Section2IconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("Edit Map Extent", "マップ範囲選択"))]]];

    // マップ範囲選択
    TWeakPtr<SPLATEAUExtentEditButton> ExtentEditButton;
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(84, 0, 86, 10)
        [SAssignNew(ExtentEditButton, SPLATEAUExtentEditButton)
        .SourcePath_Lambda(
            [this]() {
                return SourcePath;
            })
        .ZoneID_Lambda(
            [this]() {
                return ZoneID;
            })];

    TWeakPtr<SVerticalBox> PerFeatureSettingsVerticalBox;
    ModelDataPlacementVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        [SAssignNew(PerFeatureSettingsVerticalBox, SVerticalBox)
        .Visibility_Lambda(
            [ExtentEditButton] {
                return ExtentEditButton.Pin()->IsExtentSet()
                    ? EVisibility::Visible
                    : EVisibility::Collapsed;
            })];

    // 地物別設定(ヘッダー)
    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .Padding(FMargin(0, 10, 0, 10))
        .AutoHeight()
        [SNew(SHeader)
        .Content()
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(SImage)
        .Image(Style->GetBrush("PLATEAUEditor.Section3IconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("PerFeatureSettings", "地物別設定"))]]];

    // 地物別設定
    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [SNew(SPLATEAUFeatureImportSettingsView)
        .SourcePath_Lambda(
            [this]() {
                return SourcePath;
            })
        .Extent_Lambda(
            [ExtentEditButton]() {
                return ExtentEditButton.Pin()->GetExtent().Get({});
            })];

    // モデルをインポート
    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(84, 5, 86, 20))
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                const FAssetData EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
                UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
                const auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
                const auto Loader = Cast<APLATEAUCityModelLoader>(Actor);
                Loader->Source = SourcePath;
                const auto ExtentOpt = IPLATEAUEditorModule::Get().GetExtentEditor()->GetExtent();
                if (!ExtentOpt.IsSet()) {
                    // TODO: UI表示
                    return FReply::Handled();
                }
                Loader->Extent = ExtentOpt.GetValue();

                // TODO: 無しでも動くように変更
                Loader->Extent.Min.Height = -100000;
                Loader->Extent.Max.Height = 100000;

                // 設定を登録,ロード処理実行
                Loader->ImportSettings = DuplicateObject(GetMutableDefault<UPLATEAUImportSettings>(), Loader);
                Loader->LoadAsync();

                return FReply::Handled();
            })
        .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Import Button", "モデルをインポート"))
                ]
        ];
}

TSharedRef<SVerticalBox> SPLATEAUImportPanel::CreateSourcePathSelectPanel() {
    return
        SNew(SVerticalBox)
        .Visibility_Lambda([this]() {
        return bImportFromServer ? EVisibility::Collapsed : EVisibility::Visible;
            }) +
        SVerticalBox::Slot()
                .AutoHeight()
                .Padding(FMargin(0, 0, 0, 15))
                [SNew(SHorizontalBox) +
                SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .Padding(FMargin(0, 0, 5, 0))
                .AutoWidth()
                [SNew(STextBlock)
                .Text(LOCTEXT("SelectSource", "入力フォルダ"))
                ] +
                SHorizontalBox::Slot()
                [SNew(SEditableTextBox)
                .IsReadOnly(true)
                .Text_Lambda(
                    [this] {
                        return FText::FromString(SourcePath);
                    })
                ] +
                SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(FMargin(7, 0, 0, 0))
                        [SNew(SButton)
                        .VAlign(VAlign_Center)
                        .HAlign(HAlign_Center)
                        .ForegroundColor(FColor::White)
                        .ButtonColorAndOpacity(FColor(132, 132, 132))
                        .OnClicked_Raw(this, &SPLATEAUImportPanel::OnBtnSelectFolderPathClicked)
                        .Content()
                        [SNew(STextBlock)
                        .Justification(ETextJustify::Center)
                        .Margin(FMargin(15, 0, 15, 0))
                        .Text(LOCTEXT("Ref Button", "参照..."))
                        ]]] +
                SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(FMargin(0, 0, 0, 10))
                        [SNew(SEditableTextBox)
                        .IsReadOnly(true)
                        .BackgroundColor(FColor(200, 200, 200, 255))
                        .ForegroundColor(FColor(230, 30, 30))
                        .Text(LOCTEXT("Invalid Path", "直下にudxフォルダを持つフォルダを選択してください。"))
                        .Visibility_Lambda(
                            [this] {
                                auto IsValidDatasetSet = false;
                                if (FileCollection != nullptr)
                                    IsValidDatasetSet = FileCollection->getPackages() != PredefinedCityModelPackage::None;
                                return IsValidDatasetSet
                                    ? EVisibility::Collapsed
                                    : EVisibility::Visible;
                            })
                        ];
}

TSharedRef<SVerticalBox> SPLATEAUImportPanel::CreateServerPrefectureSelectPanel() {
    return
        SNew(SVerticalBox)
        .Visibility_Lambda([this]() {
        return bImportFromServer ? EVisibility::Visible : EVisibility::Collapsed;
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
                    .Padding(FMargin(0, 10, 0, 5))
                    [SNew(SHorizontalBox) +
                    SHorizontalBox::Slot()
                    .Padding(FMargin(32, 0, 0, 0))
                    .FillWidth(10)
                    [
                        SNew(STextBlock)
                        .Justification(ETextJustify::Left)
                        .Text(LOCTEXT("Max LOD", "最大LOD"))
                    ] +
                    SHorizontalBox::Slot()
                    .Padding(FMargin(0, 0, 32, 0))
                    .FillWidth(4)
                    [//TODO : 最大LODの反映
                        SNew(SEditableTextBox)
                        .Justification(ETextJustify::Left)
                        .Text(LOCTEXT("3", "3"))
                        .IsReadOnly(true)
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(FMargin(32, 0, 32, 10))
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
                    ];
}

TSharedRef<SHorizontalBox> SPLATEAUImportPanel::CreateFileSourceSelectButton() {

    return
        SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .FillWidth(0.5f) +
        SHorizontalBox::Slot()
        [
            SNew(SBox)
            .WidthOverride(120)
        .HeightOverride(30)
        .Padding(FMargin(0, 0, 5, 0))
        [
            SNew(SButton)
            .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity_Lambda([this]() {
        return !bImportFromServer ? FColor(0, 88, 88) : FColor(132, 132, 132);
            })
        .OnClicked_Lambda([this]() {
                bImportFromServer = false;
                return FReply::Handled();
            })
                .Content()
                [
                    SNew(STextBlock)
                    .Justification(ETextJustify::Center)
                .Margin(FMargin(15, 0, 15, 0))
                .Text(LOCTEXT("Local", "ローカル"))
                ]
        ]

        ] +
        SHorizontalBox::Slot()
            [
                SNew(SBox)
                .WidthOverride(120)
            .HeightOverride(30)
            .Padding(FMargin(5, 0, 0, 0))
            [
                SNew(SButton)
                .VAlign(VAlign_Center)
            .HAlign(HAlign_Center)
            .ForegroundColor(FColor::White)
            .ButtonColorAndOpacity_Lambda([this]() {
            return bImportFromServer ? FColor(0, 88, 88) : FColor(132, 132, 132);
                })
            .OnClicked_Lambda([this]() {
                    bImportFromServer = true;
                    return FReply::Handled();
                })
                    .Content()
                    [
                        SNew(STextBlock)
                        .Justification(ETextJustify::Center)
                    .Margin(FMargin(15, 0, 15, 0))
                    .Text(LOCTEXT("Server", "サーバー"))
                    ]
            ]
            ] +
            SHorizontalBox::Slot()
                .FillWidth(0.5f);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SPLATEAUImportPanel::OnBtnSelectFolderPathClicked() {
    const void* WindowHandle = nullptr;

    IMainFrameModule& MainFrameModule = IMainFrameModule::Get();
    TSharedPtr<SWindow> MainWindow = MainFrameModule.GetParentWindow();

    if (MainWindow.IsValid() && MainWindow->GetNativeWindow().IsValid()) {
        WindowHandle = MainWindow->GetNativeWindow()->GetOSWindowHandle();
    }
    const FString DialogTitle("Select folder.");
    FString OutFolderName;

    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

    if (DesktopPlatform->OpenDirectoryDialog(
        WindowHandle,
        DialogTitle,
        SourcePath,
        OutFolderName)) {
        SourcePath = OutFolderName;
        try {
            FileCollection = UdxFileCollection::find(TCHAR_TO_UTF8(*SourcePath));
        }
        catch (...) {
            FileCollection = nullptr;
            UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *SourcePath);
        }
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
