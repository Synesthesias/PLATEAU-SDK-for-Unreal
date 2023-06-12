// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUFilterTest.h"
#include "PLATEAUModelAdjustmentFilter.h"


int64 UPLATEAUFilterTest::GetCityModelPackages(const APLATEAUInstancedCityModel* TargetCityModel) {
    return UPLATEAUModelAdjustmentFilter::GetCityModelPackages(TargetCityModel);
}

FPLATEAUPackageLod UPLATEAUFilterTest::GetMinMaxLod(const APLATEAUInstancedCityModel* TargetCityModel, const int64 Package) {
    return UPLATEAUModelAdjustmentFilter::GetMinMaxLod(TargetCityModel, Package);
}

void UPLATEAUFilterTest::ApplyFilter(APLATEAUInstancedCityModel* TargetCityModel, const int64 EnablePackage, const TMap<int64, FPLATEAUPackageLod>& PackageToLodRangeMap, const bool bShowMultiLOD, const int64 EnableCityObject) {
    return UPLATEAUModelAdjustmentFilter::ApplyFilter(TargetCityModel, EnablePackage, PackageToLodRangeMap, bShowMultiLOD, EnableCityObject);
}
