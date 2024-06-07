// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelClassificationByAttribute : public FPLATEAUModelReconstruct {

public:
    FPLATEAUModelClassificationByAttribute();
    FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const EPLATEAUMeshGranularity ReconstructType, const FString AttributeKey, const TMap<FString, UMaterialInterface*> Materials);

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) override;    
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:

    FString ClassificationAttributeKey;
    TMap<FString, UMaterialInterface*> ClassificationMaterials;
    TMap<FString, int> MaterialIDMap;

};
