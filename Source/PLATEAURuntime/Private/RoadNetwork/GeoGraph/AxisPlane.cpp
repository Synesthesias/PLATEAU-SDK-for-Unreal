#include "RoadNetwork/GeoGraph/AxisPlane.h"

FVector2D FAxisPlaneEx::GetTangent(const FVector& Vector, EAxisPlane Plane) {
    switch (Plane) {
    case EAxisPlane::XY:
        return FVector2D(Vector.X, Vector.Y);
    case EAxisPlane::XZ:
        return FVector2D(Vector.X, Vector.Z);
    case EAxisPlane::YZ:
        return FVector2D(Vector.Y, Vector.Z);
    default:
        checkf(false, TEXT("Invalid plane type"));
        return FVector2D::ZeroVector;
    }
}

FVector2D FAxisPlaneEx::ToVector2D(const FVector& Vector, EAxisPlane Plane) {
    return GetTangent(Vector, Plane);
}

FVector FAxisPlaneEx::ToVector3D(const FVector2D& Vector, EAxisPlane Plane, float A) {
    switch (Plane) {
    case EAxisPlane::XY:
        return FVector(Vector.X, Vector.Y, A);
    case EAxisPlane::XZ:
        return FVector(Vector.X, A, Vector.Y);
    case EAxisPlane::YZ:
        return FVector(A, Vector.X, Vector.Y);
    default:
        checkf(false, TEXT("Invalid plane type"));
        return FVector::ZeroVector;
    }
}

float FAxisPlaneEx::GetNormal(const FVector& Vector, EAxisPlane Plane) {
    switch (Plane) {
    case EAxisPlane::XY:
        return Vector.Z;
    case EAxisPlane::XZ:
        return Vector.Y;
    case EAxisPlane::YZ:
        return Vector.X;
    default:
        checkf(false, TEXT("Invalid plane type"));
        return 0.0f;
    }
}

FVector FAxisPlaneEx::PutNormal(const FVector& Vector, EAxisPlane Plane, float N) {
    FVector Result = Vector;
    switch (Plane) {
    case EAxisPlane::XY:
        Result.Z = N;
        break;
    case EAxisPlane::XZ:
        Result.Y = N;
        break;
    case EAxisPlane::YZ:
        Result.X = N;
        break;
    default:
        checkf(false, TEXT("Invalid plane type"));
    }
    return Result;
}

FVector FAxisPlaneEx::Put(const FVector& Vector, EAxisPlane Plane, const FVector2D& A) {
    FVector Result = Vector;
    switch (Plane) {
    case EAxisPlane::XY:
        Result.X = A.X;
        Result.Y = A.Y;
        break;
    case EAxisPlane::XZ:
        Result.X = A.X;
        Result.Z = A.Y;
        break;
    case EAxisPlane::YZ:
        Result.Y = A.X;
        Result.Z = A.Y;
        break;
    default:
        checkf(false, TEXT("Invalid plane type"));
    }
    return Result;
}

FVector FAxisPlaneEx::Make(EAxisPlane Plane, const FVector2D& V, float Normal) {
    return PutNormal(Put(FVector::ZeroVector, Plane, V), Plane, Normal);
}

FVector FAxisPlaneEx::NormalVector(EAxisPlane Plane) {
    return Make(Plane, FVector2D::ZeroVector, 1.0f);
}
