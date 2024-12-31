#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
bool FGeoGraph2D::FVector2DEquitable::Equals(const FVector2D& X, const FVector2D& Y) const {
    return (X - Y).SizeSquared() < Tolerance;
}

TArray<FVector2D> FGeoGraph2D::ComputeConvexVolume(const TArray<FVector2D>& InVertices) {
    if (InVertices.Num() <= 2) {
        return TArray<FVector2D>();
    }

    // Sort vertices by X then Y
    TArray<FVector2D> SortedVertices = InVertices;
    SortedVertices.Sort([](const FVector2D& A, const FVector2D& B) {
        if (A.X != B.X) return A.X < B.X;
        return A.Y < B.Y;
        });

    // Remove duplicates
    const FVector2DEquitable Comparer(Epsilon);
    for (int32 i = 0; i < SortedVertices.Num() - 1;) {
        if (Comparer.Equals(SortedVertices[i], SortedVertices[i + 1])) {
            SortedVertices.RemoveAt(i + 1);
        }
        else {
            ++i;
        }
    }

    // Compute upper hull
    TArray<FVector2D> Result;
    Result.Add(SortedVertices[0]);
    Result.Add(SortedVertices[1]);

    for (int32 i = 2; i < SortedVertices.Num(); i++) {
        Result.Add(SortedVertices[i]);
        while (Result.Num() >= 3 && !IsLastClockwise(Result)) {
            Result.RemoveAt(Result.Num() - 2);
        }
    }

    // Compute lower hull
    Result.Add(SortedVertices[SortedVertices.Num() - 2]);
    for (int32 i = SortedVertices.Num() - 3; i >= 0; --i) {
        Result.Add(SortedVertices[i]);
        while (Result.Num() >= 3 && !IsLastClockwise(Result)) {
            Result.RemoveAt(Result.Num() - 2);
        }
    }

    return Result;
}

bool FGeoGraph2D::IsLastClockwise(const TArray<FVector2D>& List) {
    if (List.Num() <= 2) {
        return true;
    }

    const FVector2D& V1 = List[List.Num() - 1];
    const FVector2D& V2 = List[List.Num() - 2];
    const FVector2D& V3 = List[List.Num() - 3];

    const FVector2D D1 = V1 - V2;
    const FVector2D D2 = V2 - V3;

    // Cross product in 2D
    float Cross = D1.X * D2.Y - D1.Y * D2.X;
    return Cross > 0;
}
