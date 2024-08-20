// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelClassification.h"

//マテリアル分け（タイプ）
class PLATEAURUNTIME_API FPLATEAUModelClassificationByType : public FPLATEAUModelClassification {

public:
    FPLATEAUModelClassificationByType();
    FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials);
    void SetConvertGranularity(const ConvertGranularity Granularity) override;

    std::shared_ptr<plateau::polygonMesh::Model> ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) override;
    TArray<USceneComponent*> ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) override;

protected:

    TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials;
    TMap<EPLATEAUCityObjectsType, int> MaterialIDMap;
};
