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
    TFunction<FVector2D(const FVector&)> ToVec2)
{
    TArray<FVector2D > Points2D;
    for(auto& v : Vertices)
        Points2D.Add(ToVec2(v));

    auto ConvexHull2D = ComputeConvexVolume(Points2D);

    TArray<FVector> Result;
    for (const auto& Point2D : ConvexHull2D) {
        for (const auto& Vertex : Vertices) {
            if (FVector2D::Distance(ToVec2(Vertex), Point2D) < Eps) {
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
    int32 StartIndex = FindMostLeftBottom<FVector2D>(Vertices, [](FVector2D V) {return V; });
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

float FGeoGraph2D::CalcTotalAngle(
    const TArray<FVector>& Points,
    TFunction<FVector2D(const FVector&)> ToVec2)
{
    TArray<FVector2D> Points2D;
    Points2D.Reserve(Points.Num());
    for (const auto& Point : Points)
    {
        Points2D.Add(ToVec2(Point));
    }
    return CalcTotalAngle(Points2D);
}

float FGeoGraph2D::CalcTotalAngle(const TArray<FVector2D>& Points)
{
    float TotalAngle = 0.0f;
    for (int32 i = 0; i < Points.Num(); ++i)
    {
        int32 Prev = (i + Points.Num() - 1) % Points.Num();
        int32 Next = (i + 1) % Points.Num();

        FVector2D V1 = Points[i] - Points[Prev];
        FVector2D V2 = Points[Next] - Points[i];

        float Angle = FMath::Atan2(Cross(V1, V2), FVector2D::DotProduct(V1, V2));
        TotalAngle += Angle;
    }
    return TotalAngle;
}

bool FGeoGraph2D::IsConvex(const TArray<FVector2D>& Points)
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

float FGeoGraph2D::Cross(const FVector2D& A, const FVector2D& B)
{
    return A.X * B.Y - A.Y * B.X;
}