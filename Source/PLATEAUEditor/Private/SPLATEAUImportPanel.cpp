// Fill out your copyright notice in the Description page of Project Settings.


#include "SPLATEAUImportPanel.h"

#include <plateau/dataset/city_model_package.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/dataset/dataset_source.h>
#include <stdlib.h>

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
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/SPLATEAUServerDatasetSelectPanel.h"

#define LOCTEXT_NAMESPACE "SPLATEAUImportPanel"
#define BUTTON_COLOR FSlateColor(FColor(0, 255, 255))

using namespace plateau::dataset;

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
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUImportPanel::Construct(const FArguments& InArgs, const TSharedRef<FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;

    TWeakPtr<SVerticalBox> VerticalBox;
    TWeakPtr<SVerticalBox> ModelDataPlacementVerticalBox;
    ServerPanelRef = SNew(SPLATEAUServerDatasetSelectPanel);
    ServerPanelRef->InitServerData();

    ChildSlot
        [
            SAssignNew(VerticalBox, SVerticalBox)
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

    //インポート元
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
            .Text(LOCTEXT("Import From", "インポート元"))
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
        [ServerPanelRef.ToSharedRef()]

    // モデルデータの配置
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [SAssignNew(ModelDataPlacementVerticalBox, SVerticalBox)
        .Visibility_Lambda(
            [this]() {
                auto IsValidDatasetSet = false;
                if (bImportFromServer) {
                    if (ServerPanelRef->GetDatasetAccessor() != nullptr)
                        IsValidDatasetSet = ServerPanelRef->GetDatasetAccessor()->getPackages() != PredefinedCityModelPackage::None;
                } else {
                    if (DatasetAccessor != nullptr)
                        IsValidDatasetSet = DatasetAccessor->getPackages() != PredefinedCityModelPackage::None;
                }

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
            })
                .bImportFromServer_Lambda(
                    [this]() {
                        return bImportFromServer;
                    })
                .ClientRef_Lambda(
                    [this]() {
                        return ServerPanelRef->GetClientRef();
                    })
                        .ServerDatasetID_Lambda(
                            [this]() {
                                return ServerPanelRef->GetServerDatasetID();
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
        .DatasetAccessor_Lambda(
            [this]() {
                if (bImportFromServer) {
                    return ServerPanelRef->GetDatasetAccessor();
                } else {
                    return DatasetAccessor;
                }
            })
        .Extent_Lambda(
            [this, ExtentEditButton]() {
                // 緯度経度の範囲情報
                const auto Extent = ExtentEditButton.Pin()->GetExtent().Get({}).GetNativeData();

                // 平面直角座標系への変換
                auto GeoReferenceWithoutOffset = FPLATEAUGeoReference();
                GeoReferenceWithoutOffset.ZoneID = ZoneID;
                GeoReferenceWithoutOffset.UpdateNativeData();

                const auto MinPoint = GeoReferenceWithoutOffset.GetData().project(Extent.min);
                const auto MaxPoint = GeoReferenceWithoutOffset.GetData().project(Extent.max);
                const auto NewExtentCenterRaw = (MinPoint + MaxPoint) / 2.0;
                const auto NewExtentCenter = FVector3d(NewExtentCenterRaw.x, NewExtentCenterRaw.y, NewExtentCenterRaw.z);
                if (FMath::Abs(ExtentCenter.X - NewExtentCenter.X) > 1000.0 ||
                    FMath::Abs(ExtentCenter.Y - NewExtentCenter.Y) > 1000.0 ||
                    FMath::Abs(ExtentCenter.Z - NewExtentCenter.Z) > 1000.0) {
                    ExtentCenter = NewExtentCenter;
                    ReferencePoint = ExtentCenter;
                }

                return ExtentEditButton.Pin()->GetExtent().Get({});
            })];

    //オフセット値を設定
    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(10, 5, 10, 5))
        [SNew(STextBlock)
        .Text(LOCTEXT("Offset Vector", "オフセット値(cm)を設定"))
        ];

    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(84, 5, 86, 0))
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(BUTTON_COLOR)
        .OnClicked_Lambda(
            [this]() {
                ReferencePoint = ExtentCenter;
                return FReply::Handled();
            })
        .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Set Default Offset", "範囲の中心点を入力"))
                ]
        ];

    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(30, 5, 10, 0))
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(STextBlock)
        .Text(LOCTEXT("label X", "X (東が正方向)"))
        .MinDesiredWidth(100.0f)
        ] +

        SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        [SNew(SNumericEntryBox<double>)
        .AllowSpin(false)
        .OnValueCommitted_Lambda(
            [this](double value, ETextCommit::Type TextCommitType) {
                ReferencePoint.X = value;
            })
        .Value_Lambda(
            [this]() {
                return ReferencePoint.X;
            })
        ]
        ];

    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(30, 5, 10, 0))
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(STextBlock)
        .Text(LOCTEXT("label Y", "Y (南が正方向)"))
        .MinDesiredWidth(100.0f)
        ] +

        SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        [SNew(SNumericEntryBox<double>)
        .AllowSpin(false)
        .OnValueCommitted_Lambda(
            [this](double value, ETextCommit::Type TextCommitType) {
                ReferencePoint.Y = value;
            })
        .Value_Lambda(
            [this]() {
                return ReferencePoint.Y;
            })
        ]
        ];

    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(30, 5, 10, 0))
        [SNew(SHorizontalBox) +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        [SNew(STextBlock)
        .Text(LOCTEXT("label Z", "Z (高さ)"))
        .MinDesiredWidth(100.0f)
        ] +

        SHorizontalBox::Slot()
        .FillWidth(1)
        .VAlign(VAlign_Center)
        [SNew(SNumericEntryBox<double>)
        .AllowSpin(false)
        .OnValueCommitted_Lambda(
            [this](double value, ETextCommit::Type TextCommitType) {
                if (TextCommitType == ETextCommit::Type::OnEnter)
                    ReferencePoint.Z = value;
            })
        .Value_Lambda(
            [this]() {
                return ReferencePoint.Z;
            })
        ]
        ];

    // モデルをインポート
    PerFeatureSettingsVerticalBox.Pin()->AddSlot()
        .AutoHeight()
        .Padding(FMargin(84, 20, 86, 20))
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(BUTTON_COLOR)
        .OnClicked_Lambda(
            [this]() {
                const FAssetData EmptyActorAssetData = FAssetData(APLATEAUCityModelLoader::StaticClass());
                UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
                const auto Actor = FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
                const auto Loader = Cast<APLATEAUCityModelLoader>(Actor);
                Loader->bImportFromServer = bImportFromServer;
                Loader->ClientRef = ServerPanelRef->GetClientRef();
                if (bImportFromServer) {
                    Loader->Source = ServerPanelRef->GetServerDatasetID().c_str();
                } else {
                    Loader->Source = SourcePath;
                }
                const auto& ExtentOpt = IPLATEAUEditorModule::Get().GetExtentEditor()->GetExtent();
                if (!ExtentOpt.IsSet()) {
                    // TODO: UI表示
                    return FReply::Handled();
                }
                Loader->Extent = ExtentOpt.GetValue();
                Loader->Extent.Min.Height = -100000;
                Loader->Extent.Max.Height = 100000;

                Loader->GeoReference.ReferencePoint = ReferencePoint;
                Loader->GeoReference.ZoneID = ZoneID;
                Loader->GeoReference.UpdateNativeData();

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
                                if (DatasetAccessor != nullptr)
                                    IsValidDatasetSet = DatasetAccessor->getPackages() != PredefinedCityModelPackage::None;
                                return IsValidDatasetSet
                                    ? EVisibility::Collapsed
                                    : EVisibility::Visible;
                            })
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
        return !bImportFromServer ? BUTTON_COLOR : FColor(132, 132, 132);
            })
        .OnClicked_Lambda([this]() {
                bImportFromServer = false;
                ServerPanelRef->SetPanelVisibility(bImportFromServer);
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
            return bImportFromServer ? BUTTON_COLOR : FColor(132, 132, 132);
                })
            .OnClicked_Lambda([this]() {
                    bImportFromServer = true;
                    ServerPanelRef->SetPanelVisibility(bImportFromServer);
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
            const auto InDatasetSource = DatasetSource::createLocal(TCHAR_TO_UTF8(*SourcePath));
            DatasetAccessor = InDatasetSource.getAccessor();
        }
        catch (...) {
            DatasetAccessor = nullptr;
            UE_LOG(LogTemp, Error, TEXT("Invalid source path : %s"), *SourcePath);
        }
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
#undef BUTTON_COLOR
