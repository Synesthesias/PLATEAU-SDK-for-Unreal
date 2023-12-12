// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUMeshLoaderForReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUMeshLoaderForClassification : public FPLATEAUMeshLoaderForReconstruct {

public:
    FPLATEAUMeshLoaderForClassification(const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials);
    FPLATEAUMeshLoaderForClassification(const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const bool InbAutomationTest);

protected:
    bool CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;
    UMaterialInstanceDynamic* GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) override;

private:
    //Material分け時のマテリアルリスト
    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
};
