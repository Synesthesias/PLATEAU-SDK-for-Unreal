// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "PLATEAUGeometry.h"
#include "GameFramework/Actor.h"
#include "PLATEAUCityObjectGroup.h"
#include <plateau/polygon_mesh/model.h>
#include <plateau/dataset/city_model_package.h>
#include <PLATEAUImportSettings.h>
#include "Reconstruct/PLATEAUModelReconstruct.h"


class PLATEAURUNTIME_API FPLATEAUModelReconstructForClassificationSet : public FPLATEAUModelReconstruct {

public:
    /* */
    FPLATEAUModelReconstructForClassificationSet() {}
    FPLATEAUModelReconstructForClassificationSet(APLATEAUInstancedCityModel* Actor, const EPLATEAUMeshGranularity ReconstructType) {
        CityModelActor = Actor;
        MeshGranularity = static_cast<plateau::polygonMesh::MeshGranularity>(ReconstructType);
        bDivideGrid = false;
    }
    
    TArray<USceneComponent*> ReconstructFromConvertedModelForClassificationSet(std::shared_ptr<plateau::polygonMesh::Model> Model, TArray<EPLATEAUCityObjectsType>  ClassificationTypes);

protected:
    
};
