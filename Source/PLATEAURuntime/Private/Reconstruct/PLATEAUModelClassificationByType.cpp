// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByType.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <plateau/material_adjust/material_adjuster_by_type.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include <Component/PLATEAUCityObjectGroup.h>

#include "PLATEAUExportSettings.h"
#include "PLATEAUMeshExporter.h"

using namespace plateau::granularityConvert;

FPLATEAUModelClassificationByType::FPLATEAUModelClassificationByType() {}

FPLATEAUModelClassificationByType::FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials)
{
    CityModelActor = Actor;
    ClassificationMaterials = Materials;
    bDivideGrid = false;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByType::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {
    
    FPLATEAUMeshExportOptions ExtOptions;
    ExtOptions.bExportHiddenObjects = true;
    std::shared_ptr<plateau::polygonMesh::Model> smodel = FPLATEAUMeshExporter().CreateModelFromComponents(CityModelActor, TargetCityObjects, ExtOptions);

    TArray<EPLATEAUCityObjectsType>  ClassificationTypes;
    for (auto kv : ClassificationMaterials) {
        if (kv.Value != nullptr) {
            ClassificationTypes.Add(kv.Key);
        }
    }

    //指定されたタイプのModelのSubMeshにGameMaterialIDを追加
    plateau::materialAdjust::MaterialAdjusterByType Adjuster;
    auto meshes = smodel.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {

            const auto GmlId = cityobj.second;
            const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(GmlId.c_str()));
            if (AttrInfoPtr) {
                const auto Type = AttrInfoPtr->Type;
                if (ClassificationTypes.Contains(Type)) {
                    const int MaterialID = static_cast<int>(Type);
                    citygml::CityObject::CityObjectsType PlateauType = (citygml::CityObject::CityObjectsType)UPLATEAUCityObjectBlueprintLibrary::GetTypeAsInt64(Type);
                    Adjuster.registerType(GmlId, PlateauType);
                    Adjuster.registerMaterialPattern(PlateauType, MaterialID);

                    const auto AttrInfo = *AttrInfoPtr;
                    TSet<FString> Children;
                    GetChildrenGmlIds(AttrInfo, Children);
                    for (auto ChildId : Children) {
                        Adjuster.registerType(TCHAR_TO_UTF8(*ChildId), PlateauType);
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

void FPLATEAUModelClassificationByType::SetConvertGranularity(const ConvertGranularity Granularity) {
    ConvGranularity = Granularity;
}

void FPLATEAUModelClassificationByType::SetShouldConvertGranularity(const bool shouldConv)
{
    shouldConvertGranularity = shouldConv;
}


TArray<USceneComponent*> FPLATEAUModelClassificationByType::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {

    TMap<int, UMaterialInterface*> NewClassificationMaterials;
    for (const auto& KV : ClassificationMaterials) {
        const int MaterialID = static_cast<int>(KV.Key);
        NewClassificationMaterials.Add(MaterialID, KV.Value);
    }
    FPLATEAUMeshLoaderForClassification MeshLoader(NewClassificationMaterials, false);
    return FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}
