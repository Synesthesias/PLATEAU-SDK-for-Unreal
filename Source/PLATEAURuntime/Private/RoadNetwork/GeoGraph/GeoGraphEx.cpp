#include "RoadNetwork/GeoGraph/GeoGraphEx.h"

#include "RoadNetwork/Util/IntVector2Ex.h"
#include "RoadNetwork/Util/IntVectorEx.h"

TArray<FVector> FGeoGraphEx::GetInnerLerpSegments(
    const TArray<FVector>& LeftVertices,
    const TArray<FVector>& RightVertices,
    EAxisPlane Plane,
    float P) {
    P = FMath::Clamp(P, 0.0f, 1.0f);

    auto LeftEdges  = GetEdges(LeftVertices, false);
    auto RightEdges = GetEdges(RightVertices, false);

    TArray<float> Indices;
    for (int32 i = 0; i < LeftVertices.Num(); ++i) {
        Indices.Add(static_cast<float>(i));
    }

    // Rest of the implementation...
    TArray<FVector> Result;
    // Implementation details...

    return Result;
}

TArray<FIntVector> FGeoGraphEx::GetNeighborDistance3D(int32 D) {
    const int32 W = 2 * D + 1;
    const int32 Size = W * W * W;
    const int32 Half = D;
    const int32 W2 = W * W;

    TArray<FIntVector> Result;
    Result.SetNum(Size);

    for (int32 i = 0; i < Size; i++) {
        const int32 DX = i % W - Half;
        const int32 DY = (i / W) % W - Half;
        const int32 DZ = (i / W2) - Half;
        Result[i] = FIntVector(DX, DY, DZ);
    }

    return Result;
}

TArray<FIntPoint> FGeoGraphEx::GetNeighborDistance2D(int32 D) {
    const int32 W = 2 * D + 1;
    const int32 Size = W * W;
    const int32 Half = D;

    TArray<FIntPoint> Result;
    Result.SetNum(Size);

    for (int32 i = 0; i < Size; i++) {
        const int32 DX = i % W - Half;
        const int32 DY = (i / W) % W - Half;
        Result[i] = FIntPoint(DX, DY);
    }

    return Result;
}

bool FGeoGraphEx::IsCollinear(const FVector& A, const FVector& B, const FVector& C, float DegEpsilon, float MidPointTolerance) {
    if (DegEpsilon >= 0.0f) {
        const float Deg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct((A - B).GetSafeNormal(), (C - B).GetSafeNormal())));
        if (FMath::Abs(180.0f - Deg) <= DegEpsilon)
            return true;
    }

    if (MidPointTolerance >= 0.0f) {
        const FVector Dir = (C - A).GetSafeNormal();
        const FVector Pos = A + Dir * FVector::DotProduct(B - A, Dir);
        return MidPointTolerance > 0.0f && (B - Pos).SizeSquared() <= MidPointTolerance * MidPointTolerance;
    }

    return false;
}

TMap<FVector, FVector> FGeoGraphEx::MergeVertices(const TArray<FVector>& Vertices, float CellSize, int32 MergeCellLength) {
    const float Len = CellSize;
    const int32 MergeLen = MergeCellLength;

    TMap<FIntVector, TArray<FVector>> Cells;
    FIntVector Min(INT_MAX, INT_MAX, INT_MAX);

    for (const FVector& V : Vertices) {
        FIntVector C = FIntVector(
            FMath::FloorToInt(V.X / Len),
            FMath::FloorToInt(V.Y / Len),
            FMath::FloorToInt(V.Z / Len)
        );

        if (!Cells.Contains(C)) {
            Cells.Add(C, TArray<FVector>());
        }
        Cells[C].Add(V);
        Min.X = FMath::Min(Min.X, C.X);
        Min.Y = FMath::Min(Min.Y, C.Y);
        Min.Z = FMath::Min(Min.Z, C.Z);
    }

    TArray<FIntVector> Keys;
    Cells.GetKeys(Keys);
    Keys.Sort([](const FIntVector& A, const FIntVector& B) {
        auto d = A.X - B.X;
        if (d != 0) return d < 0;
        d = A.Y - B.Y;
        if (d != 0) return d < 0;
        return A.Z < B.Z;
        });
    TMap<FVector, FVector> Result;
    const auto Del1 = GetNeighborDistance3D(1);

    for(auto& K : Keys)
    {
        if (Cells.Contains(K) == false) {
            continue;
        }

        TQueue<FIntVector> Queue;
        Queue.Enqueue(K);
        while(Queue.IsEmpty() == false)
        {
            FIntVector C;
            Queue.Dequeue(C);

            for(auto D : Del1)
            {
                auto N = C + D;
                if (N == K)
                    continue;
                if (Cells.Contains(N) == false) {
                    continue;
                }
                
                if (FIntVectorEx::Sum(FIntVectorEx::Abs(K - N)) > MergeLen)
                    continue;
                for (auto& V : Cells[N])
                    Cells[K].Add(V);
                Cells.Remove(N);
                Queue.Enqueue(N);
            }
        }
    }

    for (auto& Cell : Cells) {
        if (Cell.Value.Num() == 1) continue;

        FVector Center = FVector::ZeroVector;
        for (const auto& V : Cell.Value) {
            Center += V;
        }
        Center /= Cell.Value.Num();
        for (const auto& V : Cell.Value) {
            Result.Add(V, Center);
        }
    }

    return Result;
}

bool FGeoGraphEx::IsInInnerSide(const TOptional<FLineSegment3D>& Edge, const FVector& Direction, bool bReverse, bool bIsPrev) {
    if (!Edge.IsSet()) {
        return true;
    }

    const FVector2D EdgeDir2D = FAxisPlaneEx::GetTangent(Edge.GetValue().GetDirection(), EAxisPlane::Xy);
    const FVector2D Dir2D = FAxisPlaneEx::GetTangent(Direction, EAxisPlane::Xy);

    float Cross = EdgeDir2D.X * Dir2D.Y - EdgeDir2D.Y * Dir2D.X;
    if (!bReverse) {
        Cross = -Cross;
    }

    if (Cross > 0.0f) {
        return false;
    }

    if (FMath::IsNearlyZero(Cross)) {
        float Dot = EdgeDir2D.X * Dir2D.X + EdgeDir2D.Y * Dir2D.Y;
        if (bIsPrev) {
            Dot = -Dot;
        }
        return Dot < 0.0f;
    }

    return true;
}

bool FGeoGraphEx::CheckCollision(const FVector& A, const FVector& B, const TArray<FLineSegment3D>& Edges, float IndexF) {
    const FVector2D A2 = FAxisPlaneEx::GetTangent(A, EAxisPlane::Xy);
    const FVector2D B2 = FAxisPlaneEx::GetTangent(B, EAxisPlane::Xy);

    const int32 Index = FMath::FloorToInt(IndexF);
    const float F = IndexF - Index;
    const int32 PrevIndex = F > 0.0f ? Index : Index - 1;

    for (int32 i = 0; i < Edges.Num(); ++i) {
        if (i == Index || i == PrevIndex) {
            continue;
        }

        const auto& Edge = Edges[i];
        const FVector2D EdgeStart = FAxisPlaneEx::GetTangent(Edge.GetStart(), EAxisPlane::Xy);
        const FVector2D EdgeEnd = FAxisPlaneEx::GetTangent(Edge.GetEnd(), EAxisPlane::Xy);

        // Line segment intersection check
        const FVector2D D = EdgeEnd - EdgeStart;
        const FVector2D R = B2 - A2;
        const float DenominatorU = (D.X * R.Y - D.Y * R.X);

        if (!FMath::IsNearlyZero(DenominatorU)) {
            const FVector2D S = A2 - EdgeStart;
            const float U = (R.X * S.Y - R.Y * S.X) / DenominatorU;
            const float T = (D.X * S.Y - D.Y * S.X) / DenominatorU;

            if (U >= 0.0f && U <= 1.0f && T >= 0.0f && T <= 1.0f) {
                return true;
            }
        }
    }

    return false;
}