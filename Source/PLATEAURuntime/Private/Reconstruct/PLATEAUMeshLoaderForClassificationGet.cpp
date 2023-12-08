// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "PLATEAUMeshLoader.h"
#include "Reconstruct/PLATEAUMeshLoaderForClassificationGet.h"
#include "PLATEAUCityObjectGroup.h"

bool FPLATEAUMeshLoaderForClassificationGet::CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    return SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(type);
}
UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassificationGet::GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    return UMaterialInstanceDynamic::Create(ClassificationMaterials[type], Component);
}


void FPLATEAUMeshLoaderForClassificationGet::SetClassificationMaterials(TMap<EPLATEAUCityObjectsType, UMaterialInterface*>& Materials) {
    ClassificationMaterials = Materials;
}


