// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <plateau/material_adjust/material_adjuster_by_attr.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include "CityGML/PLATEAUAttributeValue.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include <PLATEAUMeshExporter.h>
#include <PLATEAUExportSettings.h>

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

void FPLATEAUModelClassificationByAttribute::SetShouldConvertGranularity(const bool shouldConv)
{
    shouldConvertGranularity = true;
}


std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByAttribute::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    
    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = true;
    std::shared_ptr<plateau::polygonMesh::Model> smodel = FPLATEAUMeshExporter().CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    plateau::materialAdjust::MaterialAdjusterByAttr Adjuster;
    auto meshes = smodel.get()->getAllMeshes();
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

                        Adjuster.registerAttribute(GmlId, TCHAR_TO_UTF8(*Value));
                        Adjuster.registerMaterialPattern(TCHAR_TO_UTF8(*Value), MaterialID);

                        const auto AttrInfo = *AttrInfoPtr;
                        TSet<FString> Children;
                        GetChildrenGmlIds(AttrInfo, Children);
                        for (auto ChildId : Children) {
                            Adjuster.registerAttribute(TCHAR_TO_UTF8(*ChildId), TCHAR_TO_UTF8(*Value));
                        }
                    }
                }
            }
        }
    }
    Adjuster.exec(*smodel);
    
    if(shouldConvertGranularity)
    {
        //地物単位に応じたModelを再生成
        GranularityConvertOption ConvOption(ConvGranularity, bDivideGrid ? 1 : 0);
        GranularityConverter Converter;
        std::shared_ptr<plateau::polygonMesh::Model> finalConverted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*smodel, ConvOption));   
        return finalConverted;
    } else
    {
        return smodel;
    }
    
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
