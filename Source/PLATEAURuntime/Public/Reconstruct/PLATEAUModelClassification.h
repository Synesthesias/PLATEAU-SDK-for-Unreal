// Copyright 2023 Ministry of Land, Infrastructure and Transport

#pragma once

#include "CoreMinimal.h"
#include "Reconstruct/PLATEAUModelReconstruct.h"

//FPLATEAUModelClassificationByAttribute/FPLATEAUModelClassificationByTypeのBase Class
class PLATEAURUNTIME_API FPLATEAUModelClassification : public FPLATEAUModelReconstruct {

public:
    virtual void SetConvertGranularity(const ConvertGranularity Granularity) = 0;
};
