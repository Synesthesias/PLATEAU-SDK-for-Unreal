#pragma once

#include "CoreMinimal.h"
#include "LineUtil.generated.h"

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FLine {
    GENERATED_BODY()

    FLine(){}
    FLine(const FVector& InP0, const FVector& InP1);

    UPROPERTY()
    FVector P0;

    UPROPERTY()
    FVector P1;

    float GetSqrMag() const;
    float GetMag() const;
    FVector GetDirectionA2B() const;
    FVector GetDirectionB2A() const { return (P0 - P1).GetSafeNormal(); }
    FVector GetVecA2B() const { return (P1 - P0); }
    FVector GetVecB2A() const { return (P0 - P1); }
};

USTRUCT(BlueprintType)
struct PLATEAURUNTIME_API FRay2D {
    GENERATED_BODY()

    UPROPERTY()
    FVector2D Origin;

    UPROPERTY()
    FVector2D Direction;

    FRay2D() {}
    FRay2D(const FVector2D& InOrigin, const FVector2D& InDirection)
        : Origin(InOrigin), Direction(InDirection.GetSafeNormal()) {
    }
};

struct PLATEAURUNTIME_API FLineUtil {

public:
    static constexpr float Epsilon = 1e-3f;

    UFUNCTION(BlueprintCallable)
    static float DistanceBetweenLines(const FLine& Line1, const FLine& Line2, bool& bIsParallel);

    UFUNCTION(BlueprintCallable)
    static bool Intersect(const FLine& Line0, const FLine& Line1, FVector& IntersectionPoint);

    UFUNCTION(BlueprintCallable)
    static bool ContainsPoint(const FLine& Line, const FVector& Point);

    UFUNCTION(BlueprintCallable)
    static void ClosestPoints(const FLine& Line0, const FLine& Line1, FVector& ClosestPoint1, FVector& ClosestPoint2);

    UFUNCTION(BlueprintCallable)
    static float CheckHit(const FLine& Line, float Radius, const FRay& Ray, FVector& ClosestPoint, FVector& ClosestPoint2);

    UFUNCTION(BlueprintCallable)
    static float FindClosestPoint(const FLine& Line, const FRay& Ray, FVector& ClosestPoint1, FVector& ClosestPoint2);

    UFUNCTION(BlueprintCallable)
    static bool LineIntersection(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D,
        FVector2D& Intersection, float& T1, float& T2);

    UFUNCTION(BlueprintCallable)
    static bool HalfLineSegmentIntersection(const FRay2D& HalfLine, const FVector2D& P1, const FVector2D& P2,
        FVector2D& Intersection, float& T1, float& T2);

    static bool SegmentIntersection(const FVector2D& S1St, const FVector2D& S1En,
        const FVector2D& S2St, const FVector2D& S2En,
        FVector2D& OutIntersection, float& OutT1, float& OutT2);

    static float GetLineSegmentLength(const TArray<FVector>& Vertices);

    static FVector2D GetNearestPoint(const FRay2D& Self, const FVector2D& P, float& OutT);

};
