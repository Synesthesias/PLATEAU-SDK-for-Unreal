// Copyright © 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUMeshLoaderForClassificationPreprocess.h"
#include "PLATEAUMeshLoader.h"
#include "PLATEAUCityObjectGroup.h"

bool FPLATEAUMeshLoaderForClassificationPreprocess::CheckMaterialAvailability(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {

    if (!ClassificationTypes.IsEmpty() && Component->IsA(UPLATEAUCityObjectGroup::StaticClass())) {

        auto Grp = StaticCast<UPLATEAUCityObjectGroup*>(Component);
        EPLATEAUCityObjectsType type = GetCityObjectsTypeFromComponent(Grp);

        if (ClassificationTypes.Contains(type)) {
            return true;
        }
    }
    return false;
}

UMaterialInstanceDynamic* FPLATEAUMeshLoaderForClassificationPreprocess::GetMaterialForCondition(const FSubMeshMaterialSet& SubMeshValue, UStaticMeshComponent* Component) {
    const auto SourceMaterialPath = TEXT("/PLATEAU-SDK-for-Unreal/Materials/ClassificationMaterial");
    UMaterial* Mat = Cast<UMaterial>(
        StaticLoadObject(UMaterial::StaticClass(), nullptr, SourceMaterialPath));
    auto ClassificationMaterial = UMaterialInstanceDynamic::Create(Mat, Component);

    EPLATEAUCityObjectsType type = GetCityObjectsTypeFromComponent(StaticCast<UPLATEAUCityObjectGroup*>(Component));
    float MaterialID = static_cast<float>(type);
    ClassificationMaterial->SetScalarParameterValue(FName("GameMaterialID"), MaterialID);
    auto Grp = StaticCast<UPLATEAUCityObjectGroup*>(Component);
    return ClassificationMaterial;
}


void FPLATEAUMeshLoaderForClassificationPreprocess::SetClassificationTypes(TArray<EPLATEAUCityObjectsType> Types) {
    ClassificationTypes = Types;
}

EPLATEAUCityObjectsType FPLATEAUMeshLoaderForClassificationPreprocess::GetCityObjectsTypeFromComponent(UPLATEAUCityObjectGroup* Component) {
    const auto& CityObjects = Component->GetAllRootCityObjects();
    if (CityObjects.Num() == 1) {

        const auto& CityObject = CityObjects.Last();
        return CityObject.Type;
    }
    return EPLATEAUCityObjectsType::COT_Unknown;
}

bool FPLATEAUMeshLoaderForClassificationPreprocess::UseCachedMaterial() {
    return false;
}

