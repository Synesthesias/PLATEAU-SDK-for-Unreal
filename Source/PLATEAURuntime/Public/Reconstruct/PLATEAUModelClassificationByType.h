// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"

/**
 * 地物型によるマテリアル分けを行います。
 * 類似クラスとして FPLATEAUModelClassificationByAttribute があります。
 */
class PLATEAURUNTIME_API FPLATEAUModelClassificationByType : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, UMaterialInterface* Material = nullptr);
    void SetConvertGranularity(const ConvertGranularity Granularity) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) override;
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;
    void ComposeCachedMaterialFromTarget(const TArray<UPLATEAUCityObjectGroup*>& Target) override;

protected:

    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
};
