// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"

class PLATEAURUNTIME_API FPLATEAUModelClassificationByType : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByType();
    FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials);
    void SetConvertGranularity(const ConvertGranularity Granularity) override;
    void SetShouldConvertGranularity(const bool shouldConv) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) override;
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:
    bool shouldConvertGranularity = true;
    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
};
