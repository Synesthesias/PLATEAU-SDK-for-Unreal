#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <memory>

class RnPoint  {
public:
    RnPoint();
    RnPoint(const FVector& InVertex);

    FVector Vertex;

    std::shared_ptr<RnPoint> Clone() const;

    static bool Equals(const RnPoint* X, const RnPoint* Y, float SqrMagnitudeTolerance = 0.0f);

    static bool Equals(const RnPoint& X, const RnPoint& Y, float SqrMagnitudeTolerance = 0.0f);
    static bool Equals( std::shared_ptr<const RnPoint> X,  std::shared_ptr< const RnPoint> Y, float SqrMagnitudeTolerance = 0.0f);
    operator FVector() const { return Vertex; }

    bool IsSamePoint(const RnPoint* Other, float SqrMagnitudeTolerance = 0.0f) const;
};