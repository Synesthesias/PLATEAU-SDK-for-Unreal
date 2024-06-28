// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

class PLATEAURUNTIME_API FPLATEAUModelClassification : public FPLATEAUModelReconstruct {

public:
    virtual void SetMeshGranularity(const plateau::polygonMesh::MeshGranularity Granularity) = 0;
};
