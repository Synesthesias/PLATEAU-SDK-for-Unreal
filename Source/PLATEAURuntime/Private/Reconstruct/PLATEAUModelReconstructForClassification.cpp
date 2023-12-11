// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelReconstructForClassification.h>
#include "Tasks/Task.h"
#include "Misc/DefaultValueHelper.h"

#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

#include "CityGML/PLATEAUCityGmlProxy.h"
#include <PLATEAUMeshExporter.h>
#include <PLATEAUMeshLoader.h>
#include <PLATEAUExportSettings.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassification.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstructForClassification::ConvertModelForReconstructForClassification(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {

    //最小地物単位のModelを生成
    auto OriginalMeshGranularity = MeshGranularity;
    MeshGranularity = plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
    std::shared_ptr<plateau::polygonMesh::Model> converted = FPLATEAUModelReconstruct::ConvertModelForReconstruct(TargetCityObjects);
   
    //指定されたタイプのModelのSubMeshにGameMaterialIDを追加
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        for (auto& cityobj : cityObjList) {
            const auto AttrInfoPtr = CityObjMap.Find(UTF8_TO_TCHAR(cityobj.second.c_str()));
            if (AttrInfoPtr != nullptr) {
                auto Type = AttrInfoPtr->Type;
                if (ClassificationTypes.Contains(Type)) {
                    int MaterialID = static_cast<int>(Type);
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

TArray<USceneComponent*> FPLATEAUModelReconstructForClassification::ReconstructFromConvertedModelForClassification(std::shared_ptr<plateau::polygonMesh::Model> Model, TMap<EPLATEAUCityObjectsType, UMaterialInterface*> ClassificationMaterials) {

    FPLATEAUMeshLoaderForClassification MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationMaterials(ClassificationMaterials);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
