// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUModelAdjustmentFilter.h"
#include "PLATEAUImportSettings.h"
#include "PLATEAUModelAdjustmentBuilding.h"
#include "PLATEAUModelAdjustmentRelief.h"
#include "PLATEAUModelAdjustmentVegetation.h"
#include "PLATEAUEditor/Private/Widgets/SPLATEAUFilteringPanel.h"
using namespace citygml;

/**
 * @brief フィルタリング項目のパッケージ値とタイトルのマップ取得
 * @return フィルタリング項目情報を格納したマップ
 */
TMap<int64, FText> UPLATEAUModelAdjustmentFilter::GetFilteringNames() {
    return UPLATEAUImportSettings::GetFilteringNames();
}

/**
 * @brief 選択中のPLATEAUInstancedCityModelからパッケージ情報取得
 * @param InSelection アウトライナー上で選択したPLATEAUInstancedCityModel
 * @return パッケージ情報
 */
int64 UPLATEAUModelAdjustmentFilter::GetCityModelPackages(const APLATEAUInstancedCityModel* InSelection) {
    return static_cast<int64>(InSelection->GetCityModelPackages());
}

/**
 * @brief 選択中のPLATEAUInstancedCityModelからLOD情報取得
 * @param InSelection アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param InPackage LOD取得対象のパッケージ
 * @return 対象パッケージのLOD情報を格納した構造体
 */
FPLATEAUPackageLod UPLATEAUModelAdjustmentFilter::GetMinMaxLod(const APLATEAUInstancedCityModel* InSelection, const int64 InPackage) {
    const auto [MinLOD, MaxLOD] = InSelection->GetMinMaxLod(static_cast<plateau::dataset::PredefinedCityModelPackage>(InPackage));
    return FPLATEAUPackageLod(MinLOD, MaxLOD);
}

/**
 * @brief 全オブジェクトタイプを表す数値を取得
 * @return 全オブジェクトタイプを表す数値
 */
int64 UPLATEAUModelAdjustmentFilter::GetMaxCityObjectType() {
    return static_cast<int64>(CityObject::CityObjectsType::COT_All);
}

/**
 * @brief フィルタリング実行
 * @param InSelection アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param EnablePackage 有効化パッケージ
 * @param MinLOD 最小LOD
 * @param MaxLOD 最大LODの
 * @param bShowMultiLOD 重複する地物を非表示にするか？
 * @param EnableCityObject 有効化オブジェクトタイプ
 */
void UPLATEAUModelAdjustmentFilter::ApplyFilter(APLATEAUInstancedCityModel* InSelection, const int64 EnablePackage, const int MinLOD, const int MaxLOD, const bool bShowMultiLOD, const int64 EnableCityObject) {
    // オプションにない地物タイプは全て含める
    auto FilteringFlags = UPLATEAUModelAdjustmentBuilding::GetAllBuildingSettingFlags();
    FilteringFlags.Append(UPLATEAUModelAdjustmentRelief::GetAllReliefSettingFlags());
    FilteringFlags.Append(UPLATEAUModelAdjustmentVegetation::GetAllVegetationSettingFlags());
    int64 HiddenFeatureTypes = 0;
    for (const auto& FilteringFlag : FilteringFlags) {
        HiddenFeatureTypes += ~FilteringFlag;
    }
    
    InSelection->FilterByLODs(static_cast<plateau::dataset::PredefinedCityModelPackage>(EnablePackage), MinLOD, MaxLOD, bShowMultiLOD)->FilterByFeatureTypes(static_cast<CityObject::CityObjectsType>(EnableCityObject | HiddenFeatureTypes));
}
