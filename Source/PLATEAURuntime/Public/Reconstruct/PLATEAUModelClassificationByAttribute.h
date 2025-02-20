// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"
#include <plateau/material_adjust/material_adjuster_by_attr.h>

/**
 * 属性によるマテリアル分けを行います。
 * 類似クラスとして FPLATEAUModelClassificationByType があります。
 */
class PLATEAURUNTIME_API FPLATEAUModelClassificationByAttribute : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const FString& AttributeKey, const TMap<FString, UMaterialInterface*>& Materials, UMaterialInterface* Material = nullptr);
    void SetConvertGranularity(const ConvertGranularity Granularity) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) override;
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;
    void ComposeCachedMaterialFromTarget(const TArray<UPLATEAUCityObjectGroup*>& Target) override;

protected:

    FString ClassificationAttributeKey;
    TMap<FString, UMaterialInterface*> ClassificationMaterials;
};

