// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForClassification : public FPLATEAUMeshLoaderForReconstruct {

public:
    FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials);
    FPLATEAUMeshLoaderForClassification(const TMap<int, UMaterialInterface*> Materials, const bool InbAutomationTest);

protected:
    UMaterialInstanceDynamic* GetMaterialForSubMesh(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component, const FLoadInputData& LoadInputData, UTexture2D* Texture) override;
private:

    //Material分け時のマテリアルリスト
    TMap<int, UMaterialInterface*> ClassificationMaterials;
};
