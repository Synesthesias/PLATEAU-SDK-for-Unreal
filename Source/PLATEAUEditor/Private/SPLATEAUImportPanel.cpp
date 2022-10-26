// Fill out your copyright notice in the Description page of Project Settings.


#include "SPLATEAUImportPanel.h"

#include <plateau/io/mesh_convert_options.h>
#include <plateau/udx/city_model_package.h>

#include "PLATEAUCityModelLoader.h"
#include "PLATEAUImportSettings.h"

#include "AssetSelection.h"
#include "DesktopPlatformModule.h"
#include "PLATEAUEditor.h"
#include "PLATEAUEditorStyle.h"
#include "PLATEAUFeatureImportSettingsDetails.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SHeader.h"
#include "SlateOptMacros.h"
#include "ExtentEditor/PLATEAUExtentEditor.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"

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
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPLATEAUImportPanel::Construct(const FArguments& InArgs, const TSharedRef<FPLATEAUEditorStyle>& InStyle) {
    OwnerWindow = InArgs._OwnerWindow;
    Style = InStyle;

    FPropertyEditorModule& PropertyEditorModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bAllowSearch = false;
    
    BuildingImportSettingsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    BuildingImportSettingsView->RegisterInstancedCustomPropertyLayout(
        UPLATEAUImportSettings::StaticClass(),
        FOnGetDetailCustomizationInstance::CreateStatic(&FPLATEAUFeatureSettingsDetails::MakeInstance));
    BuildingImportSettingsView->SetObject(GetMutableDefault<UPLATEAUImportSettings>());
    
    ChildSlot
        [SNew(SVerticalBox)
        // ロゴ
        + SVerticalBox::Slot()
        .AutoHeight()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Center)
        [SNew(SBorder)
        .BorderImage(Style->GetBrush(TEXT("PLATEAUEditor.LogoBackground")))
        .VAlign(VAlign_Fill)
        .HAlign(HAlign_Center)
        [SNew(SImage)
        .Image(Style->GetBrush("PLATEAUEditor.LogoImage"))
        ]]

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
    +SVerticalBox::Slot()
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

    // 都市の追加
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [CreateSourcePathSelectPanel()]

    // モデルデータの配置(ヘッダー)
    +SVerticalBox::Slot()
        .Padding(FMargin(0, 15, 0, 5))
        [SNew(SHeader)
        .HAlign(HAlign_Center)
        .Content()
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading1")
        .Text(LOCTEXT("Place ModelData", "モデルデータの配置を行います。"))]]

    // 基準座標系の選択(ヘッダー)
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
        .Image(Style->GetBrush("PLATEAUEditor.Section1IconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("Select ReferenceCoordinate", "基準座標系の選択"))]]]

    // 基準座標系の選択
    + SVerticalBox::Slot()
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
                //.ButtonStyle(FEditorStyle::Get(), "ToggleButton")
                //.ForegroundColor(FSlateColor::UseForeground())
                .VAlign(VAlign_Center)
                .ButtonContent()
                [SNew(STextBlock).Text_Lambda(
                    [this]() {
                        // TODO
                        return GetZoneIDTexts()[ZoneID];
                    })]]]

    // マップ範囲選択(ヘッダー)
    + SVerticalBox::Slot()
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
        .Text(LOCTEXT("Edit Map Extent", "マップ範囲選択"))]]]

    // マップ範囲選択
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(84, 0, 86, 10)
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .HAlign(HAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Lambda(
            [this]() {
                IPLATEAUEditorModule::Get().GetExtentEditor()->SetSourcePath(SourcePath);

                // TODO: ExtentEditorに委譲
                // ビューポートの操作性向上のため100分の1スケールで設定
                const plateau::geometry::GeoReference RawGeoReference(ZoneID, {}, 1, plateau::geometry::CoordinateSystem::NWU);
                IPLATEAUEditorModule::Get().GetExtentEditor()->SetGeoReference(RawGeoReference);

                const TSharedRef<FGlobalTabmanager> GlobalTabManager = FGlobalTabmanager::Get();
                GlobalTabManager->TryInvokeTab(FPLATEAUExtentEditor::TabId);

                return FReply::Handled();
            })
        .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Margin(FMargin(80, 14, 80, 14))
                .Text(LOCTEXT("Edit Extent Button", "範囲選択"))
                ]
        ]

    // 地物別設定(ヘッダー)
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
        .Image(Style->GetBrush("PLATEAUEditor.Section3IconImage"))] +
        SHorizontalBox::Slot()
        .AutoWidth()
        .VAlign(VAlign_Center)
        .Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
        [SNew(STextBlock)
        .TextStyle(Style, "PLATEAUEditor.Heading2")
        .Text(LOCTEXT("PerFeatureSettings", "地物別設定"))]]]

    // 地物別設定
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 0)
        [BuildingImportSettingsView.ToSharedRef()]

    // モデルをインポート
    + SVerticalBox::Slot()
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

                //UE_LOG(LogTemp, Log, TEXT("%d"), FeatureSettingsMap[PredefinedCityModelPackage::Building].MaxLod);

                // 設定を登録,ロード処理実行
                Loader->ImportSettings = DuplicateObject(GetMutableDefault<UPLATEAUImportSettings>(), Loader);
                Loader->Load();

                return FReply::Handled();
            })
        .Content()
                [SNew(STextBlock)
                .Justification(ETextJustify::Center)
                .Margin(FMargin(0, 5, 0, 5))
                .Text(LOCTEXT("Import Button", "モデルをインポート"))
                ]
        ]

        ];
}

TSharedRef<SVerticalBox> SPLATEAUImportPanel::CreateSourcePathSelectPanel() {
    return
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 15))
        [SNew(SEditableTextBox)
        .Padding(FMargin(3, 3, 0, 3))
        .IsReadOnly(true)
        .Text(LOCTEXT("SelectSource", "入力元フォルダを選択"))
        .BackgroundColor(FColor(200, 200, 200, 255))
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(20, 5, 20, 5))
        [SNew(SButton)
        .VAlign(VAlign_Center)
        .ForegroundColor(FColor::White)
        .ButtonColorAndOpacity(FColor(10, 90, 80, 255))
        .OnClicked_Raw(this, &SPLATEAUImportPanel::OnBtnSelectGmlFileClicked)
        .Content()
        [SNew(STextBlock)
        .Justification(ETextJustify::Center)
        .Margin(FMargin(0, 5, 0, 5))
        .Text(LOCTEXT("Ref Button", "参照..."))
        ]
        ]
    + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(0, 0, 0, 10))
        [
            SNew(SEditableTextBox)
            .IsReadOnly(true)
        .Text_Lambda(
            [this]() {
                return FText::FromString(SourcePath);
            })
        ];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SPLATEAUImportPanel::OnBtnSelectGmlFileClicked() {
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
        //UpdateWindow(MyWindow);
    }

    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
