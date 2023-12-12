// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForClassification.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUCityObjectGroup.h"

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials) {
    ClassificationMaterials = Materials;
    bAutomationTest = false;
}

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const bool InbAutomationTest) {
    ClassificationMaterials = Materials;
    bAutomationTest = InbAutomationTest;
}

bool FPLATEAUMeshLoaderForClassification::CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    return SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(type);
}

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassification::GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    return UMaterialInstanceDynamic::Create(ClassificationMaterials[type], Component);
}


