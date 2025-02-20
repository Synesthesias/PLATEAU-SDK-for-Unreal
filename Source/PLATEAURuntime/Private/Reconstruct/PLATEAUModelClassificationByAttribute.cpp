// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByAttribute.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <plateau/material_adjust/material_adjuster_by_attr.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>

#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"
#include "CityGML/PLATEAUAttributeValue.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "Util/PLATEAUGmlUtil.h"

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


FPLATEAUModelClassificationByAttribute::FPLATEAUModelClassificationByAttribute(APLATEAUInstancedCityModel* Actor, const FString& AttributeKey, const TMap<FString, UMaterialInterface*>& Materials, UMaterialInterface* Material)
{
    CityModelActor = Actor;
    ClassificationAttributeKey = AttributeKey;
    ClassificationMaterials = Materials;
    bDivideGrid = false;
    DefaultMaterial = Material;
}

void FPLATEAUModelClassificationByAttribute::SetConvertGranularity(const ConvertGranularity Granularity) {
    ConvGranularity = Granularity;
}

void FPLATEAUModelClassificationByAttribute::ComposeCachedMaterialFromTarget(const TArray<UPLATEAUCityObjectGroup*>& Targets) {

    if (DefaultMaterial == nullptr) {
        FPLATEAUModelReconstruct::ComposeCachedMaterialFromTarget(Targets);
    }
    else {
        CachedMaterials.Clear();
        CachedMaterials.SetDefaultMaterial(DefaultMaterial);
    }
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByAttribute::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) {

    if(TargetCityObjects.Num() == 0) return nullptr;
    
    auto currentGranularity = TargetCityObjects[0]->GetConvertGranularity();
    

    //属性情報を覚えておきます。
    CityObjMap = FPLATEAUReconstructUtil::CreateMapFromCityObjectGroups(TargetCityObjects);
    
    // 現在の都市モデルをC++のModelに変換
    FPLATEAUMeshExporter MeshExporter;
    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = false;
    ExtOptions.bExportTexture = true;
    ExtOptions.TransformType = EMeshTransformType::Local;
    ExtOptions.CoordinateSystem = ECoordinateSystem::ESU;
    std::shared_ptr<plateau::polygonMesh::Model> converted = MeshExporter.CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    plateau::materialAdjust::MaterialAdjusterByAttr Adjuster;

    // CachedMaterialに元々のマテリアルを追加
    ComposeCachedMaterialFromTarget(TargetCityObjects);
    
    // ChachedMaterialに入っている元々のマテリアルに追加で、マテリアル分け用のマテリアルを追加
    TMap<int, UMaterialInterface*> ClassifyMatIDs;
    for (const auto& [Attr, Mat] : ClassificationMaterials)
    {
        if (Mat == nullptr) continue;
        int id = CachedMaterials.Add(Mat);
        Adjuster.registerMaterialPattern(TCHAR_TO_UTF8(*Attr), id);
    }

    // 変更が必要な属性値とマテリアルIDをC++側に登録
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
                    if (ClassificationMaterials.Contains(Value) && ClassificationMaterials[Value] != nullptr) {

                        Adjuster.registerAttribute(GmlId, TCHAR_TO_UTF8(*Value));

                        const auto AttrInfo = *AttrInfoPtr;
                        TSet<FString> Children = FPLATEAUGmlUtil::GetChildrenGmlIds(AttrInfo);
                        for (auto ChildId : Children) {
                            Adjuster.registerAttribute(TCHAR_TO_UTF8(*ChildId), TCHAR_TO_UTF8(*Value));
                        }
                    }
                }
            }
        }
    }
    Adjuster.exec(*converted);
    
    //地物単位に応じたModelを再生成
    if(currentGranularity != ConvGranularity)
    {
        GranularityConvertOption ConvOption(ConvGranularity, bDivideGrid ? 1 : 0);
        GranularityConverter Converter;
        std::shared_ptr<plateau::polygonMesh::Model> finalConverted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*converted, ConvOption));
        converted = finalConverted;
    }
       
    return converted;
}

TArray<USceneComponent*> FPLATEAUModelClassificationByAttribute::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {

    FPLATEAUMeshLoaderForClassification MeshLoader(CachedMaterials, false);
    return FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}
