#pragma once

#include "CoreMinimal.h"
#include "AxisPlane.h"
#include "LineSegment3D.h"
#include "Math/Vector.h"
#include "Containers/Array.h"

class PLATEAURUNTIME_API FGeoGraphEx {
public:
    template<typename T>
    static TArray<TTuple<T, T>> GetEdges(const TArray<T>& Vertices, bool bIsLoop);

    static TArray<FVector> GetInnerLerpSegments(
        const TArray<FVector>& LeftVertices,
        const TArray<FVector>& RightVertices,
        EAxisPlane Plane,
        float P);


    static TArray<FIntVector> GetNeighborDistance3D(int32 D);
    static TArray<FIntPoint> GetNeighborDistance2D(int32 D);

    static  bool IsCollinear(const FVector& A, const FVector& B, const FVector& C, float DegEpsilon, float MidPointTolerance);
    static TMap<FVector, FVector> MergeVertices(const TArray<FVector>& Vertices, float CellSize, int32 MergeCellLength);
private:
    static bool IsInInnerSide(const TOptional<FLineSegment3D>& Edge, const FVector& Direction, bool bReverse, bool bIsPrev);
    static bool CheckCollision(const FVector& A, const FVector& B, const TArray<FLineSegment3D>& Edges, float IndexF);
};

