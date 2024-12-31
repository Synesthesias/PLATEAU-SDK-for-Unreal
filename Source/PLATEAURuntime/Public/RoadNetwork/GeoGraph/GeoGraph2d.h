#pragma once

#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"

class PLATEAURUNTIME_API FGeoGraph2D {
public:
    static constexpr float Epsilon = 1e-5f;

    struct FVector2DEquitable {
        float Tolerance;

        FVector2DEquitable(float InTolerance) : Tolerance(InTolerance) {}

        bool Equals(const FVector2D& X, const FVector2D& Y) const;
    };

    static TArray<FVector> ComputeConvexVolume(const TArray<FVector>& Vertices, const FVector2D& (*ToVec2)(const FVector&));

    static TArray<FVector2D> ComputeConvexVolume(const TArray<FVector2D>& Vertices);

    static bool IsClockwise(const TArray<FVector2D>& Vertices);

    static bool Contains(const TArray<FVector2D>& Vertices, const FVector2D& Point);

private:
    static bool IsLastClockwise(const TArray<FVector2D>& List);
};
