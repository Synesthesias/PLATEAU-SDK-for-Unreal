// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUFilterReliefTest.h"
#include "PLATEAUModelAdjustmentRelief.h"


bool UPLATEAUFilterReliefTest::IsReliefPackage(const int64 Package) {
    return UPLATEAUModelAdjustmentRelief::IsReliefPackage(Package);
}

TArray<int64> UPLATEAUFilterReliefTest::GetAllReliefSettingFlags() {
    return UPLATEAUModelAdjustmentRelief::GetAllReliefSettingFlags();
}
