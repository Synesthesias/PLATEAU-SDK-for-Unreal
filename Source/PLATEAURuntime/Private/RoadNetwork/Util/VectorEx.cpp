// Plugins/PLATEAU-SDK-for-Unreal/Source/PLATEAURuntime/Private/RoadNetwork/Util/Vector2Ex.cpp
#include "RoadNetwork/Util/VectorEx.h"

FVector FVectorEx::Clamp(const FVector& Vec, float Min, float Max) {
    return FVector(FMath::Clamp(Vec.X, Min, Max), FMath::Clamp(Vec.Y, Min, Max), FMath::Clamp(Vec.Z, Min, Max));
}

FVector FVectorEx::Clamp(const FVector& Vec, const FVector& Min, const FVector& Max) {
    return FVector(FMath::Clamp(Vec.X, Min.X, Max.X), FMath::Clamp(Vec.Y, Min.Y, Max.Y), FMath::Clamp(Vec.Z, Min.Z, Max.Z));
}

FVector FVectorEx::PutX(const FVector& Vec, float X) {
    return FVector(X, Vec.Y, Vec.Z);
}

FVector FVectorEx::PutXY(const FVector& Vec, float X, float Y) {
    return FVector(X, Y, Vec.Z);
}

FVector FVectorEx::PutXZ(const FVector& Vec, float X, float Z) {
    return FVector(X, Vec.Y, Z);
}

FVector FVectorEx::PutY(const FVector& Vec, float Y) {
    return FVector(Vec.X, Y, Vec.Z);
}

FVector FVectorEx::PutYZ(const FVector& Vec, float Y, float Z) {
    return FVector(Vec.X, Y, Z);
}

FVector FVectorEx::PutZ(const FVector& Vec, float Z) {
    return FVector(Vec.X, Vec.Y, Z);
}

FVector FVectorEx::PutXY(const FVector& Vec, const FVector2D& XY) {
    return FVector(XY.X, XY.Y, Vec.Z);
}

FVector2D FVectorEx::XY(const FVector& Vec) {
    return FVector2D(Vec.X, Vec.Y);
}

FVector2D FVectorEx::XZ(const FVector& Vec) {
    return FVector2D(Vec.X, Vec.Z);
}

FVector2D FVectorEx::YZ(const FVector& Vec) {
    return FVector2D(Vec.Y, Vec.Z);
}

float FVectorEx::Min(const FVector& Vec) {
    return FMath::Min(Vec.Z, FMath::Min(Vec.X, Vec.Y));
}

float FVectorEx::Max(const FVector& Vec) {
    return FMath::Max(Vec.Z, FMath::Max(Vec.X, Vec.Y));
}

float FVectorEx::Sum(const FVector& Vec) {
    return Vec.X + Vec.Y + Vec.Z;
}

float FVectorEx::Average(const FVector& Vec) {
    return (Vec.X + Vec.Y + Vec.Z) / 3.0f;
}

FVector FVectorEx::Abs(const FVector& Vec) {
    return FVector(FMath::Abs(Vec.X), FMath::Abs(Vec.Y), FMath::Abs(Vec.Z));
}

FVector FVectorEx::Scaled(const FVector& Vec, const FVector& Mul) {
    return FVector(Vec.X * Mul.X, Vec.Y * Mul.Y, Vec.Z * Mul.Z);
}

FVector FVectorEx::RevScaled(const FVector& Vec, const FVector& Div) {
    return FVector(Vec.X / Div.X, Vec.Y / Div.Y, Vec.Z / Div.Z);
}

FVector FVectorEx::RevScale(const FVector& A, const FVector& B) {
    return FVector(A.X / B.X, A.Y / B.Y, A.Z / B.Z);
}

FIntVector FVectorEx::ToIntVector(const FVector& Vec) {
    return FIntVector(FMath::TruncToInt(Vec.X), FMath::TruncToInt(Vec.Y), FMath::TruncToInt(Vec.Z));
}

FIntVector FVectorEx::CeilToInt(const FVector& Vec) {
    return FIntVector(FMath::CeilToInt(Vec.X), FMath::CeilToInt(Vec.Y), FMath::CeilToInt(Vec.Z));
}

FIntVector FVectorEx::FloorToInt(const FVector& Vec) {
    return FIntVector(FMath::FloorToInt(Vec.X), FMath::FloorToInt(Vec.Y), FMath::FloorToInt(Vec.Z));
}

bool FVectorEx::Between(const FVector& Vec, const FVector& Min, const FVector& Max) {
    return Min.X <= Vec.X && Vec.X <= Max.X &&
        Min.Y <= Vec.Y && Vec.Y <= Max.Y &&
        Min.Z <= Vec.Z && Vec.Z <= Max.Z;
}

FVector4 FVectorEx::XYZA(const FVector& Vec, float A) {
    return FVector4(Vec.X, Vec.Y, Vec.Z, A);
}

FVector FVectorEx::Centroid(const TArray<FVector>& Points) {
    FVector Sum = FVector::ZeroVector;
    int32 Count = Points.Num();

    for (const FVector& Point : Points) {
        Sum += Point;
    }

    return Count > 0 ? Sum / Count : Sum;
}

FVector FVectorEx::AxisSymmetric(const FVector& Vec, const FVector& Axis) {
    FVector NormalizedAxis = Axis.GetSafeNormal();
    FVector OrthogonalComponent = Vec - FVector::DotProduct(Vec, NormalizedAxis) * NormalizedAxis;
    return Vec - 2 * OrthogonalComponent;
}
