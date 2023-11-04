// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ModelAdjustment/PLATEAUModelAdjustmentRelief.h"
#include "PLATEAUImportSettings.h"
using namespace citygml;

/**
 * @brief パッケージが土地起伏かどうか取得
 * @param Package パッケージ
 * @return 土地起伏か？
 */
bool UPLATEAUModelAdjustmentRelief::IsReliefPackage(const int64 Package) {
    return static_cast<plateau::dataset::PredefinedCityModelPackage>(Package) == plateau::dataset::PredefinedCityModelPackage::Relief;
}

/**
 * @brief 土地起伏のオブジェクトタイプ一覧を取得
 * @return 土地起伏のオブジェクトタイプ一覧
 */
TArray<int64> UPLATEAUModelAdjustmentRelief::GetAllReliefSettingFlags() {
    return {static_cast<int64>(CityObject::CityObjectsType::COT_TINRelief), static_cast<int64>(CityObject::CityObjectsType::COT_MassPointRelief)};
}
