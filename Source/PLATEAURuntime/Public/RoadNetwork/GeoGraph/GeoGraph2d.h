#pragma once

#include "CoreMinimal.h"
#include "Math/Vector2D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "RoadNetwork/RnDef.h"

class PLATEAURUNTIME_API FGeoGraph2D {
public:
    static constexpr float Epsilon = 1e-5f;

    struct FVector2DEquitable {
        float Tolerance;

        FVector2DEquitable(float InTolerance) : Tolerance(InTolerance) {}

        bool Equals(const FVector2D& X, const FVector2D& Y) const;
    };

    static TArray<FVector> ComputeConvexVolume(const TArray<FVector>& Vertices, const FVector2D& (*ToVec2)(const FVector&));

    static bool IsClockwise(const TArray<FVector2D>& Vertices);

    static bool Contains(const TArray<FVector2D>& Vertices, const FVector2D& Point);

    template<typename T>
    static TArray<T> ComputeConvexVolume(
        const TArray<T>& Vertices,
        TFunction<FVector(const T&)> ToVec3,
        EAxisPlane Plane,
        float SameLineTolerance = 0.0f);

    static TArray<FVector> ComputeConvexVolume(
        const TArray<FVector>& Vertices,
        TFunction<FVector2D(const FVector&)> ToVec2);

    static TArray<FVector2D> ComputeConvexVolume(const TArray<FVector2D>& Vertices);

    static TArray<int32> GetNearVertexTable(
        const TArray<FVector>& Vertices,
        TFunction<float(const FVector&, const FVector&)> CalcDistance,
        float Epsilon = 0.1f);

    static TArray<FVector> ComputeMeshOutlineVertices(
        const TArray<FVector>& Vert,
        const TArray<int32>& Triangles,
        TFunction<FVector2D(const FVector&)> ToVec2,
        float Epsilon = 0.1f);

    class FComputeOutlineResult {
    public:
        TArray<FVector> Outline;
        bool Success;
        bool HasSelfCrossing;
    };

    static FComputeOutlineResult ComputeOutline(
        const TArray<FVector>& Vertices,
        TFunction<FVector(const FVector&)> ToVec3,
        EAxisPlane Plane,
        TFunction<TArray<FVector>(const FVector&)> GetNeighbor);

    template<class T>
    static void RemoveSelfCrossing(
        TArray<T>& Points,
        TFunction<FVector2D(T)> GetTangent,
        TFunction<T(T, T, T, T, const FVector&, float, float)> CreateIntersectionPoint);

    static bool HasSelfCrossing(
        const TArray<FVector>& Points,
        TFunction<FVector2D(const FVector&)> ToVec2,
        float Epsilon = 1e-5f);

    static bool HasSelfCrossing(
        const TArray<FVector2D>& Points,
        float Epsilon = 1e-5f);

    static bool TryGetSelfCrossing(
        const TArray<FVector>& Points,
        TFunction<FVector2D(const FVector&)> ToVec2,
        int32& OutI1,
        int32& OutI2,
        int32& OutI3,
        int32& OutI4,
        FVector& OutIntersection,
        float& OutF1,
        float& OutF2,
        float Epsilon = 1e-5f);

    static bool TryGetSelfCrossing(
        const TArray<FVector2D>& Points,
        int32& OutI1,
        int32& OutI2,
        int32& OutI3,
        int32& OutI4,
        FVector2D& OutIntersection,
        float& OutF1,
        float& OutF2,
        float Epsilon = 1e-5f);

    static float CalcTotalAngle(const TArray<FVector>& Points, TFunction<FVector2D(const FVector&)> ToVec2);
    static float CalcTotalAngle(const TArray<FVector2D>& Points);

private:
    static bool IsConvex(const TArray<FVector2D>& Points);
    static int32 FindMostLeftBottom(const TArray<FVector2D>& Points);
    static float Cross(const FVector2D& A, const FVector2D& B);
    static bool IsCross(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4);
    static bool TryGetCrossPoint(const FVector2D& P1, const FVector2D& P2, const FVector2D& P3, const FVector2D& P4, FVector2D& OutIntersection, float& OutF1, float& OutF2);
   
private:
    static bool IsLastClockwise(const TArray<FVector2D>& List);
};

template<typename T>
TArray<T> FGeoGraph2D::ComputeConvexVolume(
    const TArray<T>& Vertices,
    TFunction<FVector(const T&)> ToVec3,
    EAxisPlane Plane,
    float SameLineTolerance) {
    return ComputeConvexVolume(
        Vertices.Map([&](const T& V) { return ToVec3(V); }),
        [Plane](const FVector& V) { return FAxisPlaneEx::ToVector2D(V, Plane); });
}

template<typename T>
void FGeoGraph2D::RemoveSelfCrossing(
    TArray<T>& Points,
    TFunction<FVector2D(T)> GetTangent,
    TFunction<T(T, T, T, T, const FVector&, float, float)> CreateIntersectionPoint)
{
    if (!Points || Points.Num() < 4) {
        return;
    }

    bool Found;
    do {
        Found = false;
        for (int32 i = 0; i < Points.Num() - 1; ++i) {
            for (int32 j = i + 2; j < Points.Num() - 1; ++j) {
                auto P1 = (*Points)[i];
                auto P2 = (*Points)[i + 1];
                auto P3 = (*Points)[j];
                auto P4 = (*Points)[j + 1];

                FVector2D T1 = GetTangent(P1);
                FVector2D T2 = GetTangent(P3);

                if (IsCross(T1, T2, GetTangent(P2), GetTangent(P4))) {
                    FVector2D Intersection;
                    float F1, F2;
                    if (TryGetCrossPoint(T1, T2, GetTangent(P2), GetTangent(P4), Intersection, F1, F2)) {
                        Found = true;
                        auto NewPoint = CreateIntersectionPoint(P1, P2, P3, P4, FVector(Intersection.X, Intersection.Y, 0), F1, F2);

                        // Remove points between i+1 and j
                        Points.RemoveAt(i + 1, j - i - 1);
                        Points.Insert(NewPoint, i + 1);
                        break;
                    }
                }
            }
            if (Found) break;
        }
    } while (Found);
}