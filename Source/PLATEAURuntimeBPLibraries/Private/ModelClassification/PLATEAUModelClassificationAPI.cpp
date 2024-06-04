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

    TSet<FString> GetAllAttrKeysInComponent(USceneComponent* Component) {
        TSet<FString> UniqueKeys;
        if (Component->IsA(UPLATEAUCityObjectGroup::StaticClass()) && Component->IsVisible()) {
            auto CompCityObj = StaticCast<UPLATEAUCityObjectGroup*>(Component);
            if (CompCityObj->GetStaticMesh() != nullptr) {
                for (const auto CityObj : CompCityObj->GetAllRootCityObjects()) {
                    //if (CityObj.Children.Num() == 0) {

                        //UniqueTypes.Add(CityObj.Type);
                        for (auto& attr : CityObj.Attributes.AttributeMap){

                            UniqueKeys.Emplace(attr.Key);
                        }
                        
                    //}



                    for (const auto child : CityObj.Children) {
                        //UniqueTypes.Add(child.Type);
                        //child.Attributes.AttributeMap.GetKeys(UniqueKeys);

                        /*
                        if (child.Attributes.AttributeMap.Contains("key")) {
                            const auto& val = child.Attributes.AttributeMap["key"];
                            UniqueKeys.Emplace(val.StringValue);
                        }
                        */

                        for (auto& attr : child.Attributes.AttributeMap) {

                            UniqueKeys.Emplace(attr.Key);
                        }
                    }
                }
            }
        }
        return UniqueKeys;
    }
}

TSet<FString> UPLATEAUModelClassificationAPI::SearchAttributeKeys(const TArray<USceneComponent*> TargetComponents) {
    TSet<FString> UniqueKeys;
    for (const auto comp : TargetComponents) {
        if (comp->IsA(UActorComponent::StaticClass()) || comp->IsA(UStaticMeshComponent::StaticClass()) && StaticCast<UStaticMeshComponent*>(comp)->GetStaticMesh() == nullptr && comp->IsVisible()) {
            UniqueKeys.Append(GetAllAttrKeysInComponent(comp));
            TArray<USceneComponent*> children;
            comp->GetChildrenComponents(true, children);
            for (const auto child : children) {
                UniqueKeys.Append(GetAllAttrKeysInComponent(child));
            }
        }
    }
    return UniqueKeys;
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

void UPLATEAUModelClassificationAPI::ClassifyByType(APLATEAUInstancedCityModel* TargetCityModel, TArray<USceneComponent*> TargetComponents, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials, const EPLATEAUMeshGranularity ReconstructType, bool bDestroyOriginal) {
#if WITH_EDITOR
    TargetCityModel->ClassifyModel(TargetComponents, Materials, ReconstructType, bDestroyOriginal);
#else
    FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("この機能は、エディタのみでご利用いただけます。")));
#endif  
}