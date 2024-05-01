// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"


class PLATEAURUNTIME_API FPLATEAULandscape : public FPLATEAUModelReconstruct {

public:
    FPLATEAULandscape();
    FPLATEAULandscape(APLATEAUInstancedCityModel* Actor);

    //TArray<AActor*> CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model);
    void CreateLandscape(std::shared_ptr<plateau::polygonMesh::Model> Model);

protected:


};

