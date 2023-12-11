// Copyright 2023 Ministry of Land, Infrastructure and Transport

#include "Reconstruct/PLATEAUModelReconstructForClassificationSet.h"
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
#include <Reconstruct/PLATEAUMeshLoaderForClassificationSet.h>
#include <Reconstruct/PLATEAUMeshLoaderForClassificationGet.h>

using namespace UE::Tasks;
using namespace plateau::granularityConvert;

TArray<USceneComponent*> FPLATEAUModelReconstructForClassificationSet::ReconstructFromConvertedModelForClassificationSet(std::shared_ptr<plateau::polygonMesh::Model> Model, TArray<EPLATEAUCityObjectsType>  ClassificationTypes) {
    FPLATEAUMeshLoaderForClassificationSet MeshLoader(false);

    // マテリアル分けのマテリアル設定
    MeshLoader.SetClassificationTypes(ClassificationTypes);
    return ReconstructFromConvertedModel(MeshLoader, Model);
}
