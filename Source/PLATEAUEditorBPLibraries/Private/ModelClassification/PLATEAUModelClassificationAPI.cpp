// Copyright © 2023 Ministry of Land, Infrastructure and Transport


#include "ModelClassification/PLATEAUModelClassificationAPI.h"
#include "CityGML/PLATEAUCityObject.h"
#include "PLATEAUCityObjectGroup.h"
#include "PLATEAURuntime/Public/PLATEAUInstancedCityModel.h"

namespace {

    TSet<EPLATEAUCityObjectsType> GetAllTypesInComponent(USceneComponent* Component) {
        TSet<EPLATEAUCityObjectsType> UniqueTypes;
        if (Component->IsA(UPLATEAUCityObjectGroup::StaticClass()) && Component->IsVisible()) {
            auto CompCityObj = StaticCast<UPLATEAUCityObjectGroup*>(Component);
            if (CompCityObj->GetStaticMesh() != nullptr) {
                for (const auto CityObj : CompCityObj->GetAllRootCityObjects()) {
                    if(CityObj.Children.Num() == 0)
                        UniqueTypes.Add(CityObj.Type);
                    for (const auto child : CityObj.Children) {
                        UniqueTypes.Add(child.Type);
                    }
                }
            }
        }
        return UniqueTypes;
    }
}

TSet<EPLATEAUCityObjectsType> UPLATEAUModelClassificationAPI::SearchTypes(const TArray<USceneComponent*> TargetComponents) {

    TSet<EPLATEAUCityObjectsType> UniqueTypes;
    for (const auto comp : TargetComponents) {
        if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && StaticCast<UStaticMeshComponent*>(comp)->GetStaticMesh() == nullptr && comp->IsVisible()) {
            UniqueTypes.Append(GetAllTypesInComponent(comp));
            TArray<USceneComponent*> children;
            comp->GetChildrenComponents(true, children);
            for (const auto child : children) {
                UniqueTypes.Append(GetAllTypesInComponent(child));
            }
        }     
    }
    return UniqueTypes;
}

void UPLATEAUModelClassificationAPI::ClassifyByType(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, TMap<uint8, UMaterialInterface*> Materials, const uint8 ReconstructType, bool bDestroyOriginal) {
    TargetCityModel->ClassifyModel(TargetComponents, Materials, ReconstructType, false, bDestroyOriginal);
}