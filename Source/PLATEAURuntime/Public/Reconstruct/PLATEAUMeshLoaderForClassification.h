// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "PLATEAUMeshLoaderForReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForClassification : public FPLATEAUMeshLoaderForReconstruct {

public:
    FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials);
    FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials, const bool InbAutomationTest);

protected:
    UMaterialInterface* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture, FNodeHierarchy NodeHier) override;
private:

    //Material分け時のマテリアルリスト
    TMap<int, UMaterialInterface*> ClassificationMaterials;
};
