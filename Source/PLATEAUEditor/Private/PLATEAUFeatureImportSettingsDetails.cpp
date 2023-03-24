// Copyright © 2023 Ministry of Land、Infrastructure and Transport

#include "PLATEAUFeatureImportSettingsDetails.h"

#include <plateau/dataset/city_model_package.h>

#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "DetailWidgetRow.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SNumericEntryBox.h"

#include "PLATEAUImportSettings.h"

#define LOCTEXT_NAMESPACE "PLATEAUFeatureImportSettings"

using namespace plateau::dataset;

namespace {
    TMap<EPLATEAUMeshGranularity, FText> GetGranularityTexts() {
        TMap<EPLATEAUMeshGranularity, FText> Items;
        Items.Add(EPLATEAUMeshGranularity::PerPrimaryFeatureObject, LOCTEXT("PrimaryFeatureObject", "主要地物単位"));
        Items.Add(EPLATEAUMeshGranularity::PerAtomicFeatureObject, LOCTEXT("AtomicFeatureObject", "最小地物単位"));
        Items.Add(EPLATEAUMeshGranularity::PerCityModelArea, LOCTEXT("CityModelArea", "地域単位"));
        return Items;
    }

    FName GetFeaturePlacementSettingsPropertyName(PredefinedCityModelPackage Package) {
        switch (Package) {
        case PredefinedCityModelPackage::Building: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Building);
        case PredefinedCityModelPackage::Road: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Road);
        case PredefinedCityModelPackage::Vegetation: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Vegetation);
        case PredefinedCityModelPackage::CityFurniture: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, CityFurniture);
        case PredefinedCityModelPackage::Relief: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Relief);
        case PredefinedCityModelPackage::DisasterRisk: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, DisasterRisk);
        case PredefinedCityModelPackage::LandUse: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, LandUse);
        case PredefinedCityModelPackage::UrbanPlanningDecision: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, UrbanPlanningDecision);
        case PredefinedCityModelPackage::Unknown: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Unknown);
        default: return GET_MEMBER_NAME_CHECKED(UPLATEAUImportSettings, Unknown);
        }
    }

    TArray<PredefinedCityModelPackage> GetAllPackages() {
        return {
            PredefinedCityModelPackage::Building,
            PredefinedCityModelPackage::Road,
            PredefinedCityModelPackage::Vegetation,
            PredefinedCityModelPackage::CityFurniture,
            PredefinedCityModelPackage::Relief,
            PredefinedCityModelPackage::DisasterRisk,
            PredefinedCityModelPackage::LandUse,
            PredefinedCityModelPackage::UrbanPlanningDecision,
            PredefinedCityModelPackage::Unknown,
        };
    }
}


FPLATEAUFeatureSettingsDetails::FPLATEAUFeatureSettingsDetails(
    const plateau::dataset::PredefinedCityModelPackage InPackageMask)
    : PackageMask(InPackageMask) {}

void FPLATEAUFeatureSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) {

    TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
    DetailBuilder.GetObjectsBeingCustomized(ObjectsBeingCustomized);

    auto ImportSettings = Cast<UPLATEAUImportSettings>(ObjectsBeingCustomized[0].Get());

    for (const auto& Package : GetAllPackages()) {
        const auto PropertyName = GetFeaturePlacementSettingsPropertyName(Package);
        const auto FeatureSettingsProperty = DetailBuilder.GetProperty(PropertyName);
        DetailBuilder.HideProperty(FeatureSettingsProperty);

        if ((Package & PackageMask) == PredefinedCityModelPackage::None)
            continue;

        FName CategoryName;
        FText LocalizedCategoryName;

        switch (Package) {
        case PredefinedCityModelPackage::Building:
            CategoryName = TEXT("Building");
            LocalizedCategoryName = LOCTEXT("Building", "建築物");
            break;
        case PredefinedCityModelPackage::Road:
            CategoryName = TEXT("Road");
            LocalizedCategoryName = LOCTEXT("Road", "道路");
            break;
        case PredefinedCityModelPackage::Vegetation:
            CategoryName = TEXT("Vegetation");
            LocalizedCategoryName = LOCTEXT("Vegetation", "植生");
            break;
        case PredefinedCityModelPackage::CityFurniture:
            CategoryName = TEXT("CityFurniture");
            LocalizedCategoryName = LOCTEXT("CityFurniture", "都市設備");
            break;
        case PredefinedCityModelPackage::Relief:
            CategoryName = TEXT("Relief");
            LocalizedCategoryName = LOCTEXT("Relief", "起伏");
            break;
        case PredefinedCityModelPackage::DisasterRisk:
            CategoryName = TEXT("DisasterRisk");
            LocalizedCategoryName = LOCTEXT("DisasterRisk", "災害リスク");
            break;
        case PredefinedCityModelPackage::LandUse:
            CategoryName = TEXT("LandUse");
            LocalizedCategoryName = LOCTEXT("LandUse", "土地利用");
            break;
        case PredefinedCityModelPackage::UrbanPlanningDecision:
            CategoryName = TEXT("UrbanPlanningDecision");
            LocalizedCategoryName = LOCTEXT("UrbanPlanningDecision", "都市計画決定情報");
            break;
        case PredefinedCityModelPackage::Unknown:
            CategoryName = TEXT("Unknown");
            LocalizedCategoryName = LOCTEXT("Unknown", "その他");
            break;
        }

        IDetailCategoryBuilder& Category =
            DetailBuilder.EditCategory(CategoryName, LocalizedCategoryName);

        if (!FeatureSettingsRowMap.Find(Package)) {
            FeatureSettingsRowMap.Add(Package, FPLATEAUFeatureSettingsRow(Package));

            FeatureSettingsRowMap[Package].AddToCategory(Category, FeatureSettingsProperty);
        }
    }
}

void FPLATEAUFeatureSettingsRow::AddToCategory(IDetailCategoryBuilder& Category, TSharedPtr<IPropertyHandle> FeatureSettingsProperty) const {
    const auto PackageInfo = plateau::dataset::CityModelPackageInfo::getPredefined(Package);

    const auto ImportProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, bImport));
    const auto ImportTextureProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, bImportTexture));
    const auto SetColliderProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, bSetCollider));
    const auto MinLodProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, MinLod));
    const auto MaxLodProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, MaxLod));
    auto GranularityProperty = FeatureSettingsProperty->GetChildHandle(GET_MEMBER_NAME_CHECKED(FPLATEAUFeatureImportSettings, MeshGranularity));

    {
        int MinLodValue;
        MinLodProperty->GetValue(MinLodValue);
        if (MinLodValue < PackageInfo.minLOD()) {
            MinLodValue = PackageInfo.minLOD();
            MinLodProperty->SetValue(MinLodValue);
        }
        if (MinLodValue > PackageInfo.maxLOD()) {
            MinLodValue = PackageInfo.maxLOD();
            MinLodProperty->SetValue(MinLodValue);
        }

        int MaxLodValue;
        MaxLodProperty->GetValue(MaxLodValue);
        const int MinimumMaxLodValue = FMath::Max(PackageInfo.minLOD(), MinLodValue);
        if (MaxLodValue < MinimumMaxLodValue) {
            MaxLodValue = MinimumMaxLodValue;
            MaxLodProperty->SetValue(MaxLodValue);
        }
        if (MaxLodValue > PackageInfo.maxLOD()) {
            MaxLodValue = PackageInfo.maxLOD();
            MaxLodProperty->SetValue(MaxLodValue);
        }
    }


    // インポートする
    Category.AddCustomRow(FText::FromString(TEXT("Import")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("Import", "インポートする"))]
        .ValueContent()[ImportProperty->CreatePropertyValueWidget()];

    // テクスチャをインポートする
    if (PackageInfo.hasAppearance()) {
        Category.AddCustomRow(FText::FromString(TEXT("Import Texture")))
            .NameContent()[SNew(STextBlock).Text(LOCTEXT("Import Texture", "テクスチャをインポートする"))]
            .ValueContent()[ImportTextureProperty->CreatePropertyValueWidget()];
    }

    // MeshColliderをセットする
    //Category.AddCustomRow(FText::FromString(TEXT("Set Collider")))
    //    .NameContent()[SNew(STextBlock).Text(LOCTEXT("Set Collider", "コリジョンを有効化する"))]
    //    .ValueContent()[SetColliderProperty->CreatePropertyValueWidget()];

    if (PackageInfo.minLOD() != PackageInfo.maxLOD()) {
        // 最小LOD
        Category.AddCustomRow(FText::FromString(TEXT("Min LOD")))
            .NameContent()[SNew(STextBlock).Text(LOCTEXT("Min LOD", "最小LOD"))]
            .ValueContent()
            [SNew(SNumericEntryBox<int>)
            .AllowSpin(true)
            .MinSliderValue(PackageInfo.minLOD())
            .MaxSliderValue(PackageInfo.maxLOD())
            .OnValueChanged_Lambda(
                [MinLodProperty, MaxLodProperty](const int Value) {
                    int MaxLod;
                    MaxLodProperty->GetValue(MaxLod);
                    if (Value > MaxLod)
                        return;

                    MinLodProperty->SetValue(Value);
                })
            .Value_Lambda(
                [MinLodProperty]() {
                    int Value;
                    MinLodProperty->GetValue(Value);
                    return Value;
                })
            ];

        // 最大LOD
        Category.AddCustomRow(FText::FromString(TEXT("Max LOD")))
            .NameContent()[SNew(STextBlock).Text(LOCTEXT("Max LOD", "最大LOD"))]
            .ValueContent()
            [SNew(SNumericEntryBox<int>)
            .AllowSpin(true)
            .MinSliderValue(PackageInfo.minLOD())
            .MaxSliderValue(PackageInfo.maxLOD())
            .OnValueChanged_Lambda(
                [MinLodProperty, MaxLodProperty](const int Value) {
                    int MinLod;
                    MinLodProperty->GetValue(MinLod);
                    if (Value < MinLod)
                        return;

                    MaxLodProperty->SetValue(Value);
                })
            .Value_Lambda(
                [MaxLodProperty]() {
                    int Value;
                    MaxLodProperty->GetValue(Value);
                    return Value;
                })
            ];
    }

    // モデル結合
    Category.AddCustomRow(FText::FromString(TEXT("Granularity")))
        .NameContent()[SNew(STextBlock).Text(LOCTEXT("MeshGranularity", "モデル結合"))]
        .ValueContent()
        [SNew(SComboButton)
        .OnGetMenuContent_Lambda(
            [GranularityProperty]() {
                FMenuBuilder MenuBuilder(true, nullptr);
                const auto Items = GetGranularityTexts();
                for (auto ItemIter = Items.CreateConstIterator(); ItemIter; ++ItemIter) {
                    FText ItemText = ItemIter->Value;
                    auto Gran = ItemIter->Key;
                    FUIAction ItemAction(FExecuteAction::CreateLambda(
                        [GranularityProperty, Gran]() {
                            GranularityProperty->SetValue(static_cast<uint8>(Gran));
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
                    [GranularityProperty]() {
                        // TODO
                        const auto Texts = GetGranularityTexts();

                        if (!GranularityProperty.IsValid())
                            return Texts[EPLATEAUMeshGranularity::PerCityModelArea];

                        uint8 Out;
                        GranularityProperty->GetValue(Out);
                        return Texts[static_cast<EPLATEAUMeshGranularity>(Out)];
                    })]];
}

#undef LOCTEXT_NAMESPACE
