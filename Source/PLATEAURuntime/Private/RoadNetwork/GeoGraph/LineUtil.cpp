#include "RoadNetwork/GeoGraph/LineUtil.h"

FLine::FLine(const FVector& InP0, const FVector& InP1)
    : P0(InP0), P1(InP1) {
}

float FLine::GetSqrMag() const {
    return (P0 - P1).SizeSquared();
}

float FLine::GetMag() const {
    return (P0 - P1).Size();
}

FVector FLine::GetDirectionA2B() const {
    return (P1 - P0).GetSafeNormal();
}

bool FLineUtil::LineIntersection(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D,
    FVector2D& OutIntersection, float& OutT1, float& OutT2) {
    OutT1 = OutT2 = 0.0f;
    OutIntersection = FVector2D::ZeroVector;

    const FVector2D Deno = FVector2D(B.X - A.X, B.Y - A.Y);
    const FVector2D DenoPerp = FVector2D(-Deno.Y, Deno.X);
    const float Cross = FVector2D::DotProduct(D - C, DenoPerp);

    if (FMath::Abs(Cross) < Epsilon) {
        return false;
    }

    OutT1 = FVector2D::DotProduct(C - A, D - C) / Cross;
    OutT2 = FVector2D::DotProduct(B - A, A - C) / Cross;
    OutIntersection = FMath::Lerp(A, B, OutT1);
    return true;
}

bool FLineUtil::SegmentIntersection(const FVector2D& S1St, const FVector2D& S1En,
    const FVector2D& S2St, const FVector2D& S2En,
    FVector2D& OutIntersection, float& OutT1, float& OutT2) {
    const bool Result = LineIntersection(S1St, S1En, S2St, S2En, OutIntersection, OutT1, OutT2);
    return Result && OutT1 >= 0.0f && OutT1 <= 1.0f && OutT2 >= 0.0f && OutT2 <= 1.0f;
}

float FLineUtil::GetLineSegmentLength(const TArray<FVector>& Vertices) {
    float Length = 0.0f;
    for (int32 i = 0; i < Vertices.Num() - 1; ++i) {
        Length += (Vertices[i + 1] - Vertices[i]).Size();
    }
    return Length;
}

FVector2D FLineUtil::GetNearestPoint(const FRay2D& Self, const FVector2D& P, float& OutT) {
    const FVector2D D = Self.Direction;
    OutT = FVector2D::DotProduct(Self.Direction, P - Self.Origin);
    return Self.Origin + OutT * D;
}
