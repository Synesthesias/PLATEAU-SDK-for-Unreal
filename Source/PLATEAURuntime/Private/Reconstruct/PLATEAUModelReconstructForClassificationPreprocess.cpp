// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include <Reconstruct/PLATEAUModelReconstructForClassificationPreprocess.h>
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
#include <Reconstruct/PLATEAUMeshLoaderForClassificationPreprocess.h>
#include <plateau/dataset/i_dataset_accessor.h>
#include <plateau/granularity_convert/granularity_converter.h>
#include <citygml/citygml.h>
#include <citygml/citymodel.h>

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstructForClassificationPreprocess::ConvertModelForReconstructPreprocess(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects, const TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {

    auto OriginalMeshGranularity = MeshGranularity;
    MeshGranularity = plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;

    std::shared_ptr<plateau::polygonMesh::Model> converted = FPLATEAUModelReconstruct::ConvertModelForReconstruct(TargetCityObjects);

    
    auto meshes = converted.get()->getAllMeshes();
    for (auto& mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        //auto subMeshes = mesh->getSubMeshes();

        for (auto& cityobj : cityObjList) {

            const auto AttrInfoPtr = CityObjMap.Find(FString(cityobj.second.c_str()));
            if (AttrInfoPtr != nullptr) {
                auto Type = AttrInfoPtr->Type;
                if (ClassificationTypes.Contains(Type)) {

                    int MaterialID = static_cast<int>(Type);

                    UE_LOG(LogTemp, Error, TEXT("cityobj : %d : %d, %s => %d"), cityobj.first.primary_index, cityobj.first.atomic_index, *FString(cityobj.second.c_str()), MaterialID);

                    auto subMeshes = mesh->getSubMeshes();
                    for (auto& subMesh : subMeshes) {
                        subMesh.setGameMaterialID(MaterialID);
                    }
                    mesh->setSubMeshes(subMeshes);
                }
            }
        }
    }

    //debug
    auto meshes2 = converted.get()->getAllMeshes();
    for (auto& mesh : meshes2) {
        auto cityObjList = mesh->getCityObjectList();
        //auto subMeshes = mesh->getSubMeshes();

        for (auto& cityobj : cityObjList) {

            const auto AttrInfoPtr = CityObjMap.Find(FString(cityobj.second.c_str()));
            if (AttrInfoPtr != nullptr) {
                auto Type = AttrInfoPtr->Type;
                if (ClassificationTypes.Contains(Type)) {

                    //int MaterialID = static_cast<int>(Type);

                    //UE_LOG(LogTemp, Error, TEXT("cityobj : %d : %d, %s => %d"), cityobj.first.primary_index, cityobj.first.atomic_index, *FString(cityobj.second.c_str()), MaterialID);

                    auto subMeshes = mesh->getSubMeshes();
                    for (auto& subMesh : subMeshes) {
                        //subMesh.setGameMaterialID(MaterialID);

                        UE_LOG(LogTemp, Error, TEXT("subMesh id : %d "), subMesh.getGameMaterialID());
                    }
                }
            }
        }
    }

    MeshGranularity = OriginalMeshGranularity;
    GranularityConvertOption ConvOption(MeshGranularity, bDivideGrid ? 1 : 0);
    GranularityConverter Converter;
    std::shared_ptr<plateau::polygonMesh::Model> finalConverted = std::make_shared<plateau::polygonMesh::Model>(Converter.convert(*converted, ConvOption));
    
    return finalConverted;
}

TArray<USceneComponent*> FPLATEAUModelReconstructForClassificationPreprocess::ReconstructFromConvertedModelForClassificationPreprocess(std::shared_ptr<plateau::polygonMesh::Model> Model, TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {
    FPLATEAUMeshLoaderForClassificationPreprocess MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationTypes(ClassificationTypes);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
