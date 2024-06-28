// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"

class PLATEAURUNTIME_API FPLATEAUModelClassificationByAttribute : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByAttribute();
    FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const FString AttributeKey, const TMap<FString, UMaterialInterface*> Materials);
    void SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) override;    
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:

    FString ClassificationAttributeKey;
    TMap<FString, UMaterialInterface*> ClassificationMaterials;
    TMap<FString, int> MaterialIDMap;

};
