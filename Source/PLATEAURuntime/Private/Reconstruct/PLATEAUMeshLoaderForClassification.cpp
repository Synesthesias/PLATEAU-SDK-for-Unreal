// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForClassification.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUCityObjectGroup.h"

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials) {
    ClassificationMaterials = Materials;
    bAutomationTest = false;
}

FPLATEAUMeshLoaderForClassification::FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials, const bool InbAutomationTest) {
    ClassificationMaterials = Materials;
    bAutomationTest = InbAutomationTest;
}

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassification::GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture) {
    if (SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(SubMeshValue.GameMaterialID)) {
        const auto& MatPtr = ClassificationMaterials.Find(SubMeshValue.GameMaterialID);
        if (MatPtr != nullptr) {
            const auto& Mat = *MatPtr;
            if (Mat != nullptr)
                return UMaterialInstanceDynamic::Create(Mat, Component);
        }    
    }
    return FPLATEAUMeshLoader::GetMaterialForSubMesh(SubMeshValue, Component, LoadInputData, Texture);
}


