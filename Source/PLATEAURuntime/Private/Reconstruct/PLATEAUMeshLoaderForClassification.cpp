// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForClassification.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUCityObjectGroup.h"

bool FPLATEAUMeshLoaderForClassification::CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);


    if (SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(type)) {

        UE_LOG(LogTemp, Error, TEXT("CheckMaterialAvailability : %d %s"), SubMeshValue.GameMaterialID, *Component->GetName());

        return true;
    }


    return SubMeshValue.GameMaterialID > -1 && !ClassificationMaterials.IsEmpty() && ClassificationMaterials.Contains(type);


}
UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassification::GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto& type = StaticCast<EPLATEAUCityObjectsType>(SubMeshValue.GameMaterialID);
    return UMaterialInstanceDynamic::Create(ClassificationMaterials[type], Component);
}

void FPLATEAUMeshLoaderForClassification::SetClassificationMaterials(TMap<EPLATEAUCityObjectsType, UMaterialInterface*>& Materials) {
    ClassificationMaterials = Materials;
}


