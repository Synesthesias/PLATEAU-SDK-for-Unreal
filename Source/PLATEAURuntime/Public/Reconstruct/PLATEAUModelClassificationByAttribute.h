// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"
#include <plateau/material_adjust/material_adjuster_by_attr.h>

//マテリアル分け（属性）
class PLATEAURUNTIME_API FPLATEAUModelClassificationByAttribute : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const FString& AttributeKey, const TMap<FString, UMaterialInterface*>& Materials);
    void SetConvertGranularity(const ConvertGranularity Granularity) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) override;
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:

    FString ClassificationAttributeKey;
    TMap<FString, UMaterialInterface*> ClassificationMaterials;
    // TMap<FString, int> MaterialIDMap;
};

