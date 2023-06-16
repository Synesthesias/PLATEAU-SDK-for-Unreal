// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUFilterVegetationTest.h"
#include "PLATEAUModelAdjustmentVegetation.h"


bool UPLATEAUFilterVegetationTest::IsVegetationPackage(const int64 Package) {
    return UPLATEAUModelAdjustmentVegetation::IsVegetationPackage(Package);
}

TArray<int64> UPLATEAUFilterVegetationTest::GetAllVegetationSettingFlags() {
    return UPLATEAUModelAdjustmentVegetation::GetAllVegetationSettingFlags();
}
