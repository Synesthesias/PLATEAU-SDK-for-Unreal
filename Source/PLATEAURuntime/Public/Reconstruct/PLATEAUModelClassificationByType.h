// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelClassificationByType : public FPLATEAUModelReconstruct {

public:
    FPLATEAUModelClassificationByType();
    FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const EPLATEAUMeshGranularity ReconstructType, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials);

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) override;    
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:

    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
};
