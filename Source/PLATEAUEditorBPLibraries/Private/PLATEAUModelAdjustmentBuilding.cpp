// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUModelAdjustmentBuilding.h"
#include "PLATEAUInstancedCityModel.h"
using namespace citygml;

/**
 * @brief パッケージが建造物かどうか取得
 * @param Package パッケージ
 * @return 建造物か？
 */
bool UPLATEAUModelAdjustmentBuilding::IsBuildingPackage(const int64 Package) {
    return static_cast<plateau::dataset::PredefinedCityModelPackage>(Package) == plateau::dataset::PredefinedCityModelPackage::Building;
}

/**
 * @brief 建造物のオブジェクトタイプ一覧を取得
 * @return 建造物のオブジェクトタイプ一覧
 */
TArray<int64> UPLATEAUModelAdjustmentBuilding::GetAllBuildingSettingFlags() {
    return {
        static_cast<int64>(CityObject::CityObjectsType::COT_BuildingInstallation),
        static_cast<int64>(CityObject::CityObjectsType::COT_Door),
        static_cast<int64>(CityObject::CityObjectsType::COT_Window),
        static_cast<int64>(CityObject::CityObjectsType::COT_BuildingPart),
        static_cast<int64>(CityObject::CityObjectsType::COT_WallSurface),
        static_cast<int64>(CityObject::CityObjectsType::COT_RoofSurface),
        static_cast<int64>(CityObject::CityObjectsType::COT_GroundSurface),
        static_cast<int64>(CityObject::CityObjectsType::COT_ClosureSurface),
        static_cast<int64>(CityObject::CityObjectsType::COT_OuterCeilingSurface),
        static_cast<int64>(CityObject::CityObjectsType::COT_OuterFloorSurface)
    };
}
