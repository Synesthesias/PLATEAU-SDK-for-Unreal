#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/AxisPlane.h"


bool FGeoGraph2D::FVector2DEquitable::Equals(const FVector2D& X, const FVector2D& Y) const {
    return (X - Y).SizeSquared() < Tolerance;
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

TArray<FVector> FGeoGraph2D::ComputeConvexVolume(
    const TArray<FVector>& Vertices,
    TFunction<FVector2D(const FVector&)> ToVec2) {
    auto Points2D = Vertices.Map([&](const FVector& V) { return ToVec2(V); });
    auto ConvexHull2D = ComputeConvexVolume(Points2D);

    TArray<FVector> Result;
    for (const auto& Point2D : ConvexHull2D) {
        for (const auto& Vertex : Vertices) {
            if (FVector2D::Distance(ToVec2(Vertex), Point2D) < Epsilon) {
                Result.Add(Vertex);
                break;
            }
        }
    }
    return Result;
}
TArray<FVector2D> FGeoGraph2D::ComputeConvexVolume(const TArray<FVector2D>& Vertices) {
    if (Vertices.Num() <= 3) {
        return Vertices;
    }

    // Find the leftmost bottom point
    int32 StartIndex = FindMostLeftBottom(Vertices);
    TArray<FVector2D> Points = { Vertices[StartIndex] };
    TArray<int32> Stack = { StartIndex };

    // Sort points by angle and distance
    TArray<TTuple<int32, float>> Angles;
    for (int32 i = 0; i < Vertices.Num(); ++i) {
        if (i == StartIndex) continue;

        FVector2D Diff = Vertices[i] - Vertices[StartIndex];
        float Angle = FMath::Atan2(Diff.Y, Diff.X);
        Angles.Add(MakeTuple(i, Angle));
    }

    Angles.Sort([](const TTuple<int32, float>& A, const TTuple<int32, float>& B) {
        return A.Value < B.Value;
        });

    // Graham scan
    for (const auto& IndexAngle : Angles) {
        while (Stack.Num() >= 2) {
            FVector2D P1 = Vertices[Stack[Stack.Num() - 2]];
            FVector2D P2 = Vertices[Stack.Last()];
            FVector2D P3 = Vertices[IndexAngle.Key];

            FVector2D V1 = P2 - P1;
            FVector2D V2 = P3 - P2;

            if (Cross(V1, V2) > 0) {
                break;
            }
            Stack.Pop();
            Points.Pop();
        }

        Stack.Add(IndexAngle.Key);
        Points.Add(Vertices[IndexAngle.Key]);
    }

    return Points;
}

TArray<int32> FGeoGraph2D::GetNearVertexTable(
    const TArray<FVector>& Vertices,
    TFunction<float(const FVector&, const FVector&)> CalcDistance,
    float Epsilon) {
    TArray<int32> Result;
    Result.SetNum(Vertices.Num());

    for (int32 i = 0; i < Vertices.Num(); ++i) {
        Result[i] = i;
        float MinDistance = MAX_flt;
        int32 MinIndex = i;

        for (int32 j = 0; j < i; ++j) {
            float Distance = CalcDistance(Vertices[i], Vertices[j]);
            if (Distance < MinDistance && Distance < Epsilon) {
                MinDistance = Distance;
                MinIndex = j;
            }
        }

        Result[i] = MinIndex;
    }

    return Result;
}

TArray<FVector> FGeoGraph2D::ComputeMeshOutlineVertices(
    const TArray<FVector>& Vert,
    const TArray<int32>& Triangles,
    TFunction<FVector2D(const FVector&)> ToVec2,
    float Epsilon) {
    // Create edge list from triangles
    TArray<TTuple<int32, int32>> Edges;
    for (int32 i = 0; i < Triangles.Num(); i += 3) {
        Edges.Add(MakeTuple(Triangles[i], Triangles[i + 1]));
        Edges.Add(MakeTuple(Triangles[i + 1], Triangles[i + 2]));
        Edges.Add(MakeTuple(Triangles[i + 2], Triangles[i]));
    }

    // Count edge occurrences
    TMap<TTuple<int32, int32>, int32> EdgeCount;
    for (const auto& Edge : Edges) {
        auto NormalizedEdge = Edge.Get<0>() < Edge.Get<1>() ? Edge : MakeTuple(Edge.Get<1>(), Edge.Get<0>());
        EdgeCount.Add(NormalizedEdge, EdgeCount.FindRef(NormalizedEdge) + 1);
    }

    // Find outline edges (edges that appear only once)
    TArray<TTuple<int32, int32>> OutlineEdges;
    for (const auto& Edge : Edges) {
        auto NormalizedEdge = Edge.Get<0>() < Edge.Get<1>() ? Edge : MakeTuple(Edge.Get<1>(), Edge.Get<0>());
        if (EdgeCount[NormalizedEdge] == 1) {
            OutlineEdges.Add(Edge);
        }
    }

    // Convert to continuous outline
    TArray<FVector> Result;
    if (OutlineEdges.Num() == 0) {
        return Result;
    }

    int32 CurrentVertex = OutlineEdges[0].Get<0>();
    Result.Add(Vert[CurrentVertex]);

    while (OutlineEdges.Num() > 0) {
        bool Found = false;
        for (int32 i = 0; i < OutlineEdges.Num(); ++i) {
            if (OutlineEdges[i].Get<0>() == CurrentVertex) {
                CurrentVertex = OutlineEdges[i].Get<1>();
                Result.Add(Vert[CurrentVertex]);
                OutlineEdges.RemoveAt(i);
                Found = true;
                break;
            }
            if (OutlineEdges[i].Get<1>() == CurrentVertex) {
                CurrentVertex = OutlineEdges[i].Get<0>();
                Result.Add(Vert[CurrentVertex]);
                OutlineEdges.RemoveAt(i);
                Found = true;
                break;
            }
        }

        if (!Found) {
            break;
        }
    }

    return Result;
}

FGeoGraph2D::FComputeOutlineResult FGeoGraph2D::ComputeOutline(
    const TArray<FVector>& Vertices,
    TFunction<FVector(const FVector&)> ToVec3,
    EAxisPlane Plane,
    TFunction<TArray<FVector>(const FVector&)> GetNeighbor) {
    FComputeOutlineResult Result;
    Result.Success = false;
    Result.HasSelfCrossing = false;

    if (Vertices.Num() == 0) {
        return Result;
    }

    TSet<FVector> VisitedPoints;
    TArray<FVector> OutlinePoints;
    OutlinePoints.Add(Vertices[0]);
    VisitedPoints.Add(Vertices[0]);

    while (true) {
        auto CurrentPoint = OutlinePoints.Last();
        auto Neighbors = GetNeighbor(CurrentPoint);

        if (Neighbors.Num() == 0) {
            break;
        }

        // Find the rightmost neighbor
        FVector2D CurrentPoint2D = FAxisPlaneEx::ToVector2D(CurrentPoint, Plane);
        float MaxAngle = -PI;
        FVector NextPoint = Neighbors[0];

        for (const auto& Neighbor : Neighbors) {
            if (VisitedPoints.Contains(Neighbor)) {
                continue;
            }

            FVector2D Vec = FAxisPlaneEx::ToVector2D(Neighbor - CurrentPoint, Plane);
            float Angle = FMath::Atan2(Vec.Y, Vec.X);

            if (Angle > MaxAngle) {
                MaxAngle = Angle;
                NextPoint = Neighbor;
            }
        }

        if (VisitedPoints.Contains(NextPoint)) {
            break;
        }

        OutlinePoints.Add(NextPoint);
        VisitedPoints.Add(NextPoint);
    }

    Result.Outline = OutlinePoints;
    Result.Success = true;
    Result.HasSelfCrossing = HasSelfCrossing(OutlinePoints, [Plane](const FVector& V) { return FAxisPlaneEx::ToVector2D(V, Plane); });

    return Result;
}


bool FGeoGraph2D::HasSelfCrossing(
    const TArray<FVector>& Points,
    TFunction<FVector2D(const FVector&)> ToVec2,
    float Epsilon) {
    TArray<FVector2D> Points2D;
    Points2D.Reserve(Points.Num());
    for (const auto& Point : Points) {
        Points2D.Add(ToVec2(Point));
    }
    return HasSelfCrossing(Points2D, Epsilon);
}

bool FGeoGraph2D::HasSelfCrossing(
    const TArray<FVector2D>& Points,
    float Epsilon) {
    int32 I1, I2, I3, I4;
    FVector2D Intersection;
    float F1, F2;
    return TryGetSelfCrossing(Points, I1, I2, I3, I4, Intersection, F1, F2, Epsilon);
}

bool FGeoGraph2D::TryGetSelfCrossing(
    const TArray<FVector>& Points,
    int32& OutI1,
    int32& OutI2,
    int32& OutI3,
    int32& OutI4,
    FVector& OutIntersection,
    float& OutF1,
    float& OutF2,
    float Epsilon) {
    TArray<FVector2D> Points2D;
    Points2D.Reserve(Points.Num());
    for (const auto& Point : Points) {
        Points2D.Add(FRnDef::To2D(Point));
    }

    FVector2D Intersection2D;
    bool Result = TryGetSelfCrossing(Points2D, OutI1, OutI2, OutI3, OutI4, Intersection2D, OutF1, OutF2, Epsilon);
    OutIntersection = FVector(Intersection2D.X, Intersection2D.Y, 0.0f);
    return Result;
}

bool FGeoGraph2D::TryGetSelfCrossing(
    const TArray<FVector2D>& Points,
    int32& OutI1,
    int32& OutI2,
    int32& OutI3,
    int32& OutI4,
    FVector2D& OutIntersection,
    float& OutF1,
    float& OutF2,
    float Epsilon) {
    for (int32 i = 0; i < Points.Num() - 1; ++i) {
        for (int32 j = i + 2; j < Points.Num() - 1; ++j) {
            if (IsCross(Points[i], Points[i + 1], Points[j], Points[j + 1])) {
                if (TryGetCrossPoint(Points[i], Points[i + 1], Points[j], Points[j + 1], OutIntersection, OutF1, OutF2)) {
                    OutI1 = i;
                    OutI2 = i + 1;
                    OutI3 = j;
                    OutI4 = j + 1;
                    return true;
                }
            }
        }
    }
    return false;
}

float FGeoGraph2D::CalcTotalAngle(
    const TArray<FVector>& Points,
    TFunction<FVector2(const FVector&)> ToVec2)
{
    TArray<FVector2> Points2D;
    Points2D.Reserve(Points.Num());
    for (const auto& Point : Points)
    {
        Points2D.Add(ToVec2(Point));
    }
    return CalcTotalAngle(Points2D);
}

float FGeoGraph2D::CalcTotalAngle(const TArray<FVector2>& Points)
{
    float TotalAngle = 0.0f;
    for (int32 i = 0; i < Points.Num(); ++i)
    {
        int32 Prev = (i + Points.Num() - 1) % Points.Num();
        int32 Next = (i + 1) % Points.Num();

        FVector2 V1 = Points[i] - Points[Prev];
        FVector2 V2 = Points[Next] - Points[i];

        float Angle = FMath::Atan2(Cross(V1, V2), FVector2::DotProduct(V1, V2));
        TotalAngle += Angle;
    }
    return TotalAngle;
}

bool FGeoGraph2D::IsConvex(const TArray<FVector2>& Points)
{
    if (Points.Num() < 3) return true;

    bool IsPositive = Cross(Points[1] - Points[0], Points[2] - Points[1]) > 0;
    
    for (int32 i = 1; i < Points.Num(); ++i)
    {
        int32 Next = (i + 1) % Points.Num();
        int32 NextNext = (i + 2) % Points.Num();
        
        bool CurrentIsPositive = Cross(Points[Next] - Points[i], Points[NextNext] - Points[Next]) > 0;
        if (CurrentIsPositive != IsPositive) return false;
    }
    
    return true;
}

int32 FGeoGraph2D::FindMostLeftBottom(const TArray<FVector2>& Points)
{
    int32 Result = 0;
    for (int32 i = 1; i < Points.Num(); ++i)
    {
        if (Points[i].X < Points[Result].X ||
            (Points[i].X == Points[Result].X && Points[i].Y < Points[Result].Y))
        {
            Result = i;
        }
    }
    return Result;
}

float FGeoGraph2D::Cross(const FVector2& A, const FVector2& B)
{
    return A.X * B.Y - A.Y * B.X;
}

bool FGeoGraph2D::IsCross(const FVector2& P1, const FVector2& P2, const FVector2& P3, const FVector2& P4)
{
    float D1 = Cross(P2 - P1, P3 - P1);
    float D2 = Cross(P2 - P1, P4 - P1);
    float D3 = Cross(P4 - P3, P1 - P3);
    float D4 = Cross(P4 - P3, P2 - P3);

    return (D1 * D2 < 0) && (D3 * D4 < 0);
}

bool FGeoGraph2D::TryGetCrossPoint(
    const FVector2& P1,
    const FVector2& P2,
    const FVector2& P3,
    const FVector2& P4,
    FVector2& OutIntersection,
    float& OutF1,
    float& OutF2)
{
    FVector2 V1 = P2 - P1;
    FVector2 V2 = P4 - P3;
    float Cross12 = Cross(V1, V2);

    if (FMath::Abs(Cross12) < Epsilon)
    {
        return false;
    }

    FVector2 V3 = P3 - P1;
    OutF1 = Cross(V2, V3) / Cross12;
    OutF2 = Cross(V1, V3) / Cross12;

    if (OutF1 < 0 || OutF1 > 1 || OutF2 < 0 || OutF2 > 1)
    {
        return false;
    }

    OutIntersection = P1 + V1 * OutF1;
    return true;
}
