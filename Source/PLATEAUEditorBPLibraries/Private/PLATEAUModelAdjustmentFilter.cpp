// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUModelAdjustmentFilter.h"
#include "PLATEAUImportSettings.h"
#include "PLATEAUModelAdjustmentBuilding.h"
#include "PLATEAUModelAdjustmentRelief.h"
#include "PLATEAUModelAdjustmentVegetation.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"
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
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @return パッケージ情報
 */
int64 UPLATEAUModelAdjustmentFilter::GetCityModelPackages(const APLATEAUInstancedCityModel* TargetCityModel) {
    return static_cast<int64>(TargetCityModel->GetCityModelPackages());
}

/**
 * @brief 選択中のPLATEAUInstancedCityModelからLod情報取得
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param Package Lod取得対象のパッケージ
 * @return 対象パッケージのLod情報を格納した構造体
 */
FPLATEAUPackageLod UPLATEAUModelAdjustmentFilter::GetMinMaxLod(const APLATEAUInstancedCityModel* TargetCityModel, const int64 Package) {
    const auto [MinLod, MaxLod] = TargetCityModel->GetMinMaxLod(static_cast<plateau::dataset::PredefinedCityModelPackage>(Package));
    return FPLATEAUPackageLod(MinLod, MaxLod);
}

/**
 * @brief フィルタリング実行
 * @param TargetCityModel アウトライナー上で選択したPLATEAUInstancedCityModel
 * @param EnablePackage 有効化パッケージ
 * @param PackageToLodRangeMap パッケージごとのLodに関してのユーザー選択結果 
 * @param bOnlyMaxLod 最大のLODのみを表示するか？
 * @param EnableCityObject 有効化オブジェクトタイプ
 */
void UPLATEAUModelAdjustmentFilter::ApplyFilter(APLATEAUInstancedCityModel* TargetCityModel, const int64 EnablePackage, const TMap<int64, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bOnlyMaxLod, const int64 EnableCityObject) {
    // オプションにない地物タイプは全て含める
    auto FilteringFlags = UPLATEAUModelAdjustmentBuilding::GetAllBuildingSettingFlags();
    FilteringFlags.Append(UPLATEAUModelAdjustmentRelief::GetAllReliefSettingFlags());
    FilteringFlags.Append(UPLATEAUModelAdjustmentVegetation::GetAllVegetationSettingFlags());
    int64 HiddenFeatureTypes = 0;
    for (const auto& FilteringFlag : FilteringFlags) {
        HiddenFeatureTypes += ~FilteringFlag;
    }

    TMap<plateau::dataset::PredefinedCityModelPackage, FPLATEAUMinMaxLod> CastPackageToLodRangeMap;
    for (const auto& Entity : PackageToLodRangeMap) {
        CastPackageToLodRangeMap.Add(static_cast<plateau::dataset::PredefinedCityModelPackage>(Entity.Key), { Entity.Value.MinLod, Entity.Value.MaxLod });
    }
    TargetCityModel->FilterByLods(static_cast<plateau::dataset::PredefinedCityModelPackage>(EnablePackage), CastPackageToLodRangeMap, bOnlyMaxLod)->FilterByFeatureTypes(static_cast<CityObject::CityObjectsType>(EnableCityObject | HiddenFeatureTypes));
}
