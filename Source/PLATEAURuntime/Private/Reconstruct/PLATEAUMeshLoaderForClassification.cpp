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

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassification::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    if (SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(type)) {
        return UMaterialInstanceDynamic::Create(ClassificationMaterials[type], Component);
    }
    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture);
}


