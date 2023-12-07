// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "ModelAdjustment/PLATEAUModelAdjustmentVegetation.h"
#include "PLATEAUImportSettings.h"
using namespace citygml;

/**
 * @brief パッケージが植生かどうか取得
 * @param Package パッケージ
 * @return 植生か？
 */
bool UPLATEAUModelAdjustmentVegetation::IsVegetationPackage(const int64 Package) {
    return static_cast<plateau::dataset::PredefinedCityModelPackage>(Package) == plateau::dataset::PredefinedCityModelPackage::Vegetation;
}

/**
 * @brief 植生のオブジェクトタイプ一覧を取得
 * @return 植生のオブジェクトタイプ一覧
 */
TArray<int64> UPLATEAUModelAdjustmentVegetation::GetAllVegetationSettingFlags() {
    return {static_cast<int64>(CityObject::CityObjectsType::COT_SolitaryVegetationObject), static_cast<int64>(CityObject::CityObjectsType::COT_PlantCover)};
}
