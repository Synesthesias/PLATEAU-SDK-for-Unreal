// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUFilterBuildingTest.h"
#include "PLATEAUModelAdjustmentBuilding.h"
#include "PLATEAUInstancedCityModel.h"
using namespace citygml;


bool UPLATEAUFilterBuildingTest::IsBuildingPackage(const int64 Package) {
    return UPLATEAUModelAdjustmentBuilding::IsBuildingPackage(Package);
}

TArray<int64> UPLATEAUFilterBuildingTest::GetAllBuildingSettingFlags() {
    return UPLATEAUModelAdjustmentBuilding::GetAllBuildingSettingFlags();
}

int64 UPLATEAUFilterBuildingTest::GetBuildingPackage() {
    return static_cast<int64>(plateau::dataset::PredefinedCityModelPackage::Building);
}
