// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <plateau/material_adjust/material_adjuster_by_attr.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include "CityGML/PLATEAUAttributeValue.h"
#include "Component/PLATEAUCityObjectGroup.h"

using namespace plateau::granularityConvert;

namespace {

    TSet<FString> ConvertAttributeValuesToUniqueStringValues(TArray<FPLATEAUAttributeValue> AttributeValues) {
        TSet<FString> StringValues;
        for (const auto& AttributeValue : AttributeValues) {
            StringValues.Add(AttributeValue.StringValue);
        }
        return StringValues;
    }
}

FPLATEAUModelClassificationByAttribute::FPLATEAUModelClassificationByAttribute() {}

FPLATEAUModelClassificationByAttribute::FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const FString AttributeKey, const TMap<FString, UMaterialInterface*> Materials)
{
    CityModelActor = Actor;
    ClassificationAttributeKey = AttributeKey;
    ClassificationMaterials = Materials;
    bDivideGrid = false;

    //属性の値ごとにMaterial ID生成
    int ID = 0;
    for (const auto& KV : ClassificationMaterials) {
        MaterialIDMap.Add(KV.Key, ID);
        ID++;
    }
}

void FPLATEAUModelClassificationByAttribute::SetConvertGranularity(const ConvertGranularity Granularity) {
    ConvGranularity = Granularity;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByAttribute::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    //最小地物単位のModelを生成
    std::shared_ptr<plateau::polygonMesh::Model> converted = ConvertModelWithGranularity(TargetCityObjects, ConvertGranularity::PerAtomicFeatureObject);

    plateau::materialAdjust::MaterialAdjusterByAttr Adjuster;
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {

            const auto GmlId = cityobj.second;
            const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(GmlId.c_str()));
            if (AttrInfoPtr) {
                TArray<FPLATEAUAttributeValue> AttributeValues = UPLATEAUAttributeValueBlueprintLibrary::GetAttributesByKey(ClassificationAttributeKey, AttrInfoPtr->Attributes);
                TSet<FString> AttributeStringValues = ConvertAttributeValuesToUniqueStringValues(AttributeValues);
                
                for (const auto& Value : AttributeStringValues) {
                    if (MaterialIDMap.Contains(Value)) {
                        int MaterialID = MaterialIDMap[Value];

                        bool bRes = Adjuster.registerAttribute(GmlId, TCHAR_TO_UTF8(*Value));
                        Adjuster.registerMaterialPattern(TCHAR_TO_UTF8(*Value), MaterialID);

                        UE_LOG(LogTemp, Error, TEXT("Register: %s %s"), *FString(GmlId.c_str()), bRes ? TEXT("True") : TEXT("False"));

                        const auto AttrInfo = *AttrInfoPtr;
                        TSet<FString> Children;
                        GetChildrenGmlIds(AttrInfo, Children);
                        for (auto ChildId : Children) {
                            bool bResC = Adjuster.registerAttribute(TCHAR_TO_UTF8(*ChildId), TCHAR_TO_UTF8(*Value));

                            UE_LOG(LogTemp, Error, TEXT("Register C: %s %s"), *ChildId, bResC ? TEXT("True") : TEXT("False"));
                        }

                        //debug Sub Meshに直接
                        auto subMeshes = mesh->getSubMeshes();
                        for (auto& subMesh : subMeshes) {
                            subMesh.setGameMaterialID(MaterialID);
                        }
                        mesh->setSubMeshes(subMeshes);
                    }
                }
            }
        }
    }
    Adjuster.exec(*converted);

    //地物単位に応じたModelを再生成
    GranularityConvertOption ConvOption(ConvGranularity, bDivideGrid ? 1 : 0);
    GranularityConverter Converter;
    std::shared_ptr<plateau::polygonMesh::Model> finalConverted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*converted, ConvOption));   
    return finalConverted;
}

TArray<USceneComponent*> FPLATEAUModelClassificationByAttribute::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {

    TMap<int, UMaterialInterface*> NewClassificationMaterials;
    for (const auto& KV : ClassificationMaterials) {
        const int* MaterialID = MaterialIDMap.Find(KV.Key);
        if(MaterialID != nullptr && KV.Value != nullptr)
            NewClassificationMaterials.Add(*MaterialID, KV.Value);
    }

    FPLATEAUMeshLoaderForClassification MeshLoader(NewClassificationMaterials, false);
    return FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}
