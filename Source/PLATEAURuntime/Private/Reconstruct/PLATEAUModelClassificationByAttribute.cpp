// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include "CityGML/PLATEAUAttributeValue.h"
#include <PLATEAUCityObjectGroup.h>

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

void FPLATEAUModelClassificationByAttribute::SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity) {
    MeshGranularity = Granularity;
}


std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByAttribute::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    //最小地物単位のModelを生成
    auto OriginalMeshGranularity = MeshGranularity;
    MeshGranularity = plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
    std::shared_ptr<plateau::polygonMesh::Model> converted = FPLATEAUModelReconstruct::ConvertModelForReconstruct(TargetCityObjects);

    //指定された属性のSubMeshにGameMaterialIDを追加
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {
            const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(cityobj.second.c_str()));
            if (AttrInfoPtr != nullptr) {
                TArray<FPLATEAUAttributeValue> AttributeValues = UPLATEAUAttributeValueBlueprintLibrary::GetAttributesByKey(ClassificationAttributeKey, AttrInfoPtr->Attributes);
                TSet<FString> AttributeStringValues = ConvertAttributeValuesToUniqueStringValues(AttributeValues);
                for (const auto& Value : AttributeStringValues) {
                    if (MaterialIDMap.Contains(Value)) {
                        int MaterialID = MaterialIDMap[Value];
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

    //地物単位に応じたModelを再生成
    MeshGranularity = OriginalMeshGranularity;
    GranularityConvertOption ConvOption(MeshGranularity, bDivideGrid ? 1 : 0);
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
