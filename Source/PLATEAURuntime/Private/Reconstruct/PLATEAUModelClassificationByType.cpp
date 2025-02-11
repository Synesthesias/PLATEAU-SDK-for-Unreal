// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByType.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <plateau/material_adjust/material_adjuster_by_type.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include <Component/PLATEAUCityObjectGroup.h>
#include <Util/PLATEAUGmlUtil.h>

using namespace plateau::granularityConvert;


FPLATEAUModelClassificationByType::FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials)
{
    CityModelActor = Actor;
    ClassificationMaterials = Materials;
    bDivideGrid = false;
    
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByType::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*>& TargetCityObjects) {

    //最小地物単位のModelを生成
    std::shared_ptr<plateau::polygonMesh::Model> converted = ConvertModelWithGranularity(TargetCityObjects, ConvertGranularity::PerAtomicFeatureObject);

    //指定されたタイプのModelのSubMeshにGameMaterialIDを追加
    plateau::materialAdjust::MaterialAdjusterByType Adjuster;

    // CachedMaterialに元々のマテリアルを追加
    ComposeCachedMaterialFromTarget(TargetCityObjects);

    // ChachedMaterialに入っている元々のマテリアルに追加で、マテリアル分け用のマテリアルを追加
    TMap<int, UMaterialInterface*> ClassifyMatIDs;
    for(const auto& [Type, Mat] : ClassificationMaterials)
    {
        if(Mat == nullptr) continue;
        int id = CachedMaterials.Add(Mat);
        citygml::CityObject::CityObjectsType PlateauType = (citygml::CityObject::CityObjectsType)UPLATEAUCityObjectBlueprintLibrary::GetTypeAsInt64(Type);
        Adjuster.registerMaterialPattern(PlateauType, id);
    }

    // 変更が必要な地物型とマテリアルIDをC++側に登録
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {

            const auto GmlId = cityobj.second;


            if (const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(GmlId.c_str()))) {
                const auto Type = AttrInfoPtr->Type;
                if (ClassificationMaterials.Contains(Type) && ClassificationMaterials[Type] != nullptr) {
                    citygml::CityObject::CityObjectsType PlateauType = (citygml::CityObject::CityObjectsType)UPLATEAUCityObjectBlueprintLibrary::GetTypeAsInt64(Type);
                    Adjuster.registerType(GmlId, PlateauType);
            
                    const auto AttrInfo = *AttrInfoPtr;
                    TSet<FString> Children = FPLATEAUGmlUtil::GetChildrenGmlIds(AttrInfo);
                    for (auto ChildId : Children) {
                        Adjuster.registerType(TCHAR_TO_UTF8(*ChildId), PlateauType);
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

void FPLATEAUModelClassificationByType::SetConvertGranularity(const ConvertGranularity Granularity) {
    ConvGranularity = Granularity;
}

TArray<USceneComponent*> FPLATEAUModelClassificationByType::ReconstructFromConvertedModel(std::shared_ptr<plateau::polygonMesh::Model> Model) {

    // TMap<int, UMaterialInterface*> NewClassificationMaterials;
    // for (const auto& KV : ClassificationMaterials) {
    //     const int* MaterialID = MaterialIDMap.Find(KV.Key);
    //     if (MaterialID != nullptr && KV.Value != nullptr)
    //         NewClassificationMaterials.Add(*MaterialID, KV.Value);
    // }
    
    FPLATEAUMeshLoaderForClassification MeshLoader(CachedMaterials, false);
    return FPLATEAUModelReconstruct::ReconstructFromConvertedModelWithMeshLoader(MeshLoader, Model);
}
