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


using namespace UE::Tasks;
using namespace plateau::granularityConvert;

std::shared_ptr<plateau::polygonMesh::Model> FPLATEAUModelReconstructForClassificationPreprocess::ConvertModelForReconstruct(const TArray<UPLATEAUCityObjectGroup*> TargetCityObjects) {

    std::shared_ptr<plateau::polygonMesh::Model> converted = FPLATEAUModelReconstruct::ConvertModelForReconstruct(TargetCityObjects);

    /*
    auto meshes = converted.get()->getAllMeshes();

    for (auto mesh : meshes) {
        auto cityObjList = mesh->getCityObjectList();
        auto subMeshes = mesh->getSubMeshes();

        for (auto cityobj : cityObjList) {

            UE_LOG(LogTemp, Error, TEXT("cityobj : %d : %d, %s"), cityobj.first.primary_index, cityobj.first.atomic_index,  *FString(cityobj.second.c_str()));


        }


    }
    */
    return converted;
}

TArray<USceneComponent*> FPLATEAUModelReconstructForClassificationPreprocess::ReconstructFromConvertedModelForClassificationPreprocess(std::shared_ptr<plateau::polygonMesh::Model> Model, TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {
    FPLATEAUMeshLoaderForClassificationPreprocess MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationTypes(ClassificationTypes);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
