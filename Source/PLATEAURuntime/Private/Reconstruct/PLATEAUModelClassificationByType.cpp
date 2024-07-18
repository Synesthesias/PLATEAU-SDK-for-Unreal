// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelClassificationByType.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include <Component/PLATEAUCityObjectGroup.h>

using namespace plateau::granularityConvert;

FPLATEAUModelClassificationByType::FPLATEAUModelClassificationByType() {}

FPLATEAUModelClassificationByType::FPLATEAUModelClassificationByType(APLATEAUInstancedCityModel* Actor, const TMap<EPLATEAUCityObjectsType, UMaterialInterface*> Materials)
{
    CityModelActor = Actor;
    ClassificationMaterials = Materials;
    bDivideGrid = false;
}

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelClassificationByType::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    //最小地物単位のModelを生成
    auto OriginalMeshGranularity = MeshGranularity;
    MeshGranularity = plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
    std::shared_ptr<plateau::polygonMesh::Model> converted = FPLATEAUModelReconstruct::ConvertModelForReconstruct(TargetCityObjects);

    TArray<EPLATEAUCityObjectsType>  ClassificationTypes;
    for (auto kv : ClassificationMaterials) {
        if (kv.Value != nullptr) {
            ClassificationTypes.Add(kv.Key);
        }
    }

    //指定されたタイプのModelのSubMeshにGameMaterialIDを追加
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {
            const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(cityobj.second.c_str()));
            if (AttrInfoPtr != nullptr) {
                const auto Type = AttrInfoPtr->Type;
                if (ClassificationTypes.Contains(Type)) {
                    const int MaterialID = static_cast<int>(Type);
                    auto subMeshes = mesh->getSubMeshes();
                    for (auto& subMesh : subMeshes) {
                        subMesh.setGameMaterialID(MaterialID);
                    }
                    mesh->setSubMeshes(subMeshes);
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

void FPLATEAUModelClassificationByType::SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity) {
    MeshGranularity = Granularity;
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
