#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <memory>

#include "RoadNetwork/RnDef.h"

class RnPoint  {
public:
    RnPoint();
    RnPoint(const FVector& InVertex);

    FVector Vertex;

    RnRef_t<RnPoint> Clone() const;

    static bool Equals(const RnPoint* X, const RnPoint* Y, float SqrMagnitudeTolerance = 0.0f);

    static bool Equals(const RnPoint& X, const RnPoint& Y, float SqrMagnitudeTolerance = 0.0f);
    static bool Equals(RnRef_t<const RnPoint> X, RnRef_t< const RnPoint> Y, float SqrMagnitudeTolerance = 0.0f);
    operator FVector() const { return Vertex; }

    bool IsSamePoint(const RnPoint* Other, float SqrMagnitudeTolerance = 0.0f) const;
};
