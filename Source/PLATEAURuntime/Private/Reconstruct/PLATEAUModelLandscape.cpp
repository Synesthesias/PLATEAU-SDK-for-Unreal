// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelLandscape.h>
#include <Reconstruct/PLATEAUMeshLoaderForLandscape.h>

namespace {

    bool HasCityObjectsType(UPLATEAUCityObjectGroup* CityObj, EPLATEAUCityObjectsType Type) {
        const auto& found = CityObj->GetAllRootCityObjects().FindByPredicate([&](FPLATEAUCityObject obj) {
            return obj.Type == Type;
            });
        return found != nullptr;
    }
}

FPLATEAUModelLandscape::FPLATEAUModelLandscape() {}

FPLATEAUModelLandscape::FPLATEAUModelLandscape(APLATEAUInstancedCityModel* Actor) {
    CityModelActor = Actor;
}

TArray<HeightmapCreationResult> FPLATEAUModelLandscape::CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model, FPLATEAULandscapeParam Param) {
    FPLATEAUMeshLoaderForLandscape HMap = FPLATEAUMeshLoaderForLandscape(false);
    return HMap.CreateHeightMap(CityModelActor, Model, Param);
}

/**
* @brief ComponentのChildrenからUPLATEAUCityObjectGroupを探してtypeがTINRelief || ReliefFeatureの場合のみリストに追加します
*/
TArray<UPLATEAUCityObjectGroup*> FPLATEAUModelLandscape::GetUPLATEAUCityObjectGroupsFromSceneComponents(TArray<USceneComponent*> TargetComponents) {
    TSet<UPLATEAUCityObjectGroup*> UniqueComponents;
    for (auto comp : TargetComponents) {
        if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && StaticCast<UStaticMeshComponent*>(comp)->GetStaticMesh() == nullptr && comp->IsVisible()) {
            TArray<USceneComponent*> children;
            comp->GetChildrenComponents(true, children);
            for (auto child : children) {
                if (child->IsA(UPLATEAUCityObjectGroup::StaticClass()) && child->IsVisible()) {
                    auto childCityObj = StaticCast<UPLATEAUCityObjectGroup*>(child);
                    if (childCityObj->GetStaticMesh() != nullptr ) {
                       if(HasCityObjectsType(childCityObj, EPLATEAUCityObjectsType::COT_TINRelief) || HasCityObjectsType(childCityObj, EPLATEAUCityObjectsType::COT_ReliefFeature))
                            UniqueComponents.Add(childCityObj);
                    }
                }
            }
        }
        if (comp->IsA(UPLATEAUCityObjectGroup::StaticClass()) && comp->IsVisible()) {
            const auto& cityObj = StaticCast<UPLATEAUCityObjectGroup*>(comp);
            if (HasCityObjectsType(cityObj, EPLATEAUCityObjectsType::COT_TINRelief) || HasCityObjectsType(cityObj, EPLATEAUCityObjectsType::COT_ReliefFeature))
                UniqueComponents.Add(StaticCast<UPLATEAUCityObjectGroup*>(comp));
        }        
    }
    return UniqueComponents.Array();
}