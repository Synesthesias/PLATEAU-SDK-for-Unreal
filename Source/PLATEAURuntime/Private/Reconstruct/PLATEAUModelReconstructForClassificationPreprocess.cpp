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


using namespace UE::Tasks;
using namespace plateau::granularityConvert;

TArray<USceneComponent*> FPLATEAUModelReconstructForClassificationPreprocess::ReconstructFromConvertedModelForClassificationPreprocess(std::shared_ptr<plateau::polygonMesh::Model> Model, TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {
    FPLATEAUMeshLoaderForClassificationPreprocess MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationTypes(ClassificationTypes);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
