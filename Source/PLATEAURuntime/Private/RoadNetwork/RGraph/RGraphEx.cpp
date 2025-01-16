#include "RoadNetwork/RGraph/RGraphEx.h"

#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/CityObject/SubDividedCityObject.h"

namespace
{
}

void FRGraphEx::RemoveInnerVertex(RGraphRef_t<URFace> Face) {
    if (!Face) return;

    TArray<RGraphRef_t<URVertex>> Vertices;
    for (auto Edge : Face->GetEdges()) {
        Vertices.Add(Edge->GetV0());
        Vertices.Add(Edge->GetV1());
    }

    for (auto Vertex : Vertices) {
        if (Vertex->GetFaces().Num() == 1) {
            Vertex->DisConnect(true);
        }
    }
}

void FRGraphEx::RemoveInnerVertex(RGraphRef_t<URGraph> Graph) {
    if (!Graph) return;

    for (auto& Face : Graph->GetFaces()) {
        RemoveInnerVertex(Face);
    }
}

TSet<RGraphRef_t<URVertex>> FRGraphEx::AdjustSmallLodHeight(
    RGraphRef_t<URGraph> Graph,
    float MergeCellSize,
    int32 MergeCellLength,
    float HeightTolerance) {
    if (!Graph) return TSet<RGraphRef_t<URVertex>>();

    TSet<RGraphRef_t<URVertex>> Result;
    auto&& Vertices = Graph->GetAllVertices();

    // Create grid for vertex grouping
    TMap<FIntVector2, TArray<RGraphRef_t<URVertex>>> Grid;
    for (auto Vertex : Vertices) {
        FVector2D Pos2D = FRnDef::To2D(Vertex->Position);
        FIntVector2 GridPos(
            FMath::FloorToInt(Pos2D.X / MergeCellSize),
            FMath::FloorToInt(Pos2D.Y / MergeCellSize)
        );
        Grid.FindOrAdd(GridPos).Add(Vertex);
    }

    // Process each grid cell
    for (auto& GridPair : Grid) {
        auto& CellVertices = GridPair.Value;
        if (CellVertices.Num() <= 1) continue;

        // Group vertices by LOD level
        TMap<int32, TArray<RGraphRef_t<URVertex>>> LodGroups;
        for (auto Vertex : CellVertices) {
            int32 LodLevel = Vertex->GetMaxLodLevel();
            LodGroups.FindOrAdd(LodLevel).Add(Vertex);
        }

        // Process each LOD group
        for (auto& LodPair : LodGroups) {
            if (LodPair.Value.Num() <= 1) continue;

            float AverageHeight = 0.0f;
            for (auto Vertex : LodPair.Value) {
                AverageHeight += Vertex->Position.Z;
            }
            AverageHeight /= LodPair.Value.Num();

            for (auto Vertex : LodPair.Value) {
                if (FMath::Abs(Vertex->Position.Z - AverageHeight) <= HeightTolerance) {
                    Vertex->Position.Z = AverageHeight;
                    Result.Add(Vertex);
                }
            }
        }
    }

    return Result;
}

void FRGraphEx::VertexReduction(
    RGraphRef_t<URGraph> Graph,
    float MergeCellSize,
    int32 MergeCellLength,
    float MidPointTolerance) {
    if (!Graph) return;

    auto&& Vertices = Graph->GetAllVertices();
    TMap<FIntVector2, TArray<RGraphRef_t<URVertex>>> Grid;

    // Create grid for vertex grouping
    for (auto Vertex : Vertices) {
        FVector2D Pos2D = FRnDef::To2D(Vertex->Position);
        FIntVector2 GridPos(
            FMath::FloorToInt(Pos2D.X / MergeCellSize),
            FMath::FloorToInt(Pos2D.Y / MergeCellSize)
        );
        Grid.FindOrAdd(GridPos).Add(Vertex);
    }

    // Process each grid cell
    for (auto& GridPair : Grid) {
        auto& CellVertices = GridPair.Value;
        if (CellVertices.Num() <= 1) continue;

        // Group vertices by road type and LOD level
        TMap<TTuple<ERRoadTypeMask, int32>, TArray<RGraphRef_t<URVertex>>> TypeLodGroups;
        for (auto Vertex : CellVertices) {
            ERRoadTypeMask RoadType = Vertex->GetTypeMaskOrDefault();
            int32 LodLevel = Vertex->GetMaxLodLevel();
            TypeLodGroups.FindOrAdd(MakeTuple(RoadType, LodLevel)).Add(Vertex);
        }

        // Process each type-LOD group
        for (auto& GroupPair : TypeLodGroups) {
            auto& GroupVertices = GroupPair.Value;
            if (GroupVertices.Num() <= 1) continue;

            // Calculate average position
            FVector AveragePos(0, 0, 0);
            for (auto Vertex : GroupVertices) {
                AveragePos += Vertex->Position;
            }
            AveragePos /= GroupVertices.Num();

            // Find closest vertex to average position
            float MinDist = MAX_flt;
            RGraphRef_t<URVertex> ClosestVertex = nullptr;
            for (auto Vertex : GroupVertices) {
                float Dist = FVector::DistSquared(Vertex->Position, AveragePos);
                if (Dist < MinDist) {
                    MinDist = Dist;
                    ClosestVertex = Vertex;
                }
            }

            if (ClosestVertex) {
                ClosestVertex->Position = AveragePos;
                for (auto Vertex : GroupVertices) {
                    if (Vertex != ClosestVertex) {
                        Vertex->MergeTo(ClosestVertex);
                    }
                }
            }
        }
    }
}

void FRGraphEx::EdgeReduction(RGraphRef_t<URGraph> Graph) {
    if (!Graph) return;

    auto&& Edges = Graph->GetAllEdges();
    for (auto Edge : Edges) {
        for (auto OtherEdge : Edge->GetNeighborEdges()) {
            if (Edge->IsSameVertex(OtherEdge)) {
                Edge->MergeTo(OtherEdge);
                break;
            }
        }
    }
}

void FRGraphEx::MergeIsolatedVertices(RGraphRef_t<URGraph> Graph) {
    if (!Graph) return;

    for (auto& Face : Graph->GetFaces()) {
        MergeIsolatedVertex(Face);
    }
}

void FRGraphEx::MergeIsolatedVertex(RGraphRef_t<URFace> Face) {
    if (!Face) return;

    TArray<RGraphRef_t<URVertex>> Vertices;
    for (auto Edge : Face->GetEdges()) {
        Vertices.Add(Edge->GetV0());
        Vertices.Add(Edge->GetV1());
    }

    for (auto Vertex : Vertices) {
        if (Vertex->GetEdges().Num() == 2) {
            TArray<RGraphRef_t<UREdge>> VertexEdges = Vertex->GetEdges().Array();
            if (VertexEdges.Num() == 2) {
                RGraphRef_t<URVertex> V0 = VertexEdges[0]->GetOppositeVertex(Vertex);
                RGraphRef_t<URVertex> V1 = VertexEdges[1]->GetOppositeVertex(Vertex);

                if (V0 && V1 && !V0->IsNeighbor(V1)) {
                    auto NewEdge = RGraphNew<UREdge>(V0, V1);
                    for (auto Edge : VertexEdges) {
                        for (auto EdgeFace : Edge->GetFaces()) {
                            EdgeFace->AddEdge(NewEdge);
                        }
                    }
                    Vertex->DisConnect(true);
                }
            }
        }
    }
}

TArray<RGraphRef_t<URFaceGroup>> FRGraphEx::GroupBy(
    RGraphRef_t<URGraph> Graph,
    TFunction<bool(RGraphRef_t<URFace>, RGraphRef_t<URFace>)> IsMatch) {
    TArray<RGraphRef_t<URFaceGroup>> Result;
    if (!Graph) 
        return Result;

    TSet<RGraphRef_t<URFace>> UnprocessedFaces;
    for(auto&& Face : Graph->GetFaces())
        UnprocessedFaces.Add(Face);
    while (UnprocessedFaces.Num() > 0) {
        TArray<RGraphRef_t<URFace>> GroupFaces;
        auto FirstFace = *UnprocessedFaces.CreateIterator();
        GroupFaces.Add(FirstFace);
        UnprocessedFaces.Remove(FirstFace);

        for (int32 i = 0; i < GroupFaces.Num(); ++i) {
            auto Face = GroupFaces[i];
            TArray<RGraphRef_t<URFace>> Neighbors;
            for (auto Edge : Face->GetEdges()) {
                Neighbors.Append(Edge->GetFaces().Array());
            }

            for (auto Neighbor : Neighbors) {
                if (UnprocessedFaces.Contains(Neighbor) && IsMatch(Face, Neighbor)) {
                    GroupFaces.Add(Neighbor);
                    UnprocessedFaces.Remove(Neighbor);
                }
            }
        }

        Result.Add(RGraphNew<URFaceGroup>(Graph, FirstFace->GetCityObjectGroup().Get(), GroupFaces));
    }

    return Result;
}

void FRGraphEx::Optimize(
    RGraphRef_t<URGraph> Graph,
    float MergeCellSize,
    int32 MergeCellLength,
    float MidPointTolerance,
    float Lod1HeightTolerance) {
    if (!Graph) return;

    InsertVerticesInEdgeIntersection(Graph, Lod1HeightTolerance);
    AdjustSmallLodHeight(Graph, MergeCellSize, MergeCellLength, Lod1HeightTolerance);
    VertexReduction(Graph, MergeCellSize, MergeCellLength, MidPointTolerance);
    EdgeReduction(Graph);
    MergeIsolatedVertices(Graph);
}

void FRGraphEx::InsertVertexInNearEdge(RGraphRef_t<URGraph> Graph, float Tolerance)
{
    if (!Graph) return;

    auto&& Vertices = Graph->GetAllVertices();
    auto&& Edges = Graph->GetAllEdges();
    TArray<TTuple<RGraphRef_t<URVertex>, RGraphRef_t<UREdge>>> InsertVertices;

    // #TODO : 元の実装と違う(平面操作層ではなく全探索になっている)
#if false
    for (auto Vertex : Vertices) {
        for (auto Edge : Edges) {
            if (!Edge->GetEdges().Contains(Vertex)) {
                FLineSegment2D Segment(
                    FRnDef::To2D(Edge->GetV0()->Position),
                    FRnDef::To2D(Edge->GetV1()->Position)
                );

                float Distance = Segment.GetDistance(FRnDef::To2D(Vertex->Position));
                if (Distance <= Tolerance) {
                    InsertVertices.Add(MakeTuple(Vertex, Edge));
                }
            }
        }
    }
#endif
    for (const auto& Pair : InsertVertices) {
        Pair.Get<1>()->SplitEdge(Pair.Get<0>());
    }
}

void FRGraphEx::InsertVerticesInEdgeIntersection(RGraphRef_t<URGraph> Graph, float HeightTolerance) {
    if (!Graph) return;

    auto&& Edges = Graph->GetAllEdges().Array();
    for (int32 i = 0; i < Edges.Num(); ++i) {
        for (int32 j = i + 1; j < Edges.Num(); ++j) {
            auto Edge1 = Edges[i];
            auto Edge2 = Edges[j];

            if (!Edge1->IsShareAnyVertex(Edge2)) {
                FVector2D E1_Start = FRnDef::To2D(Edge1->GetV0()->Position);
                FVector2D E1_End = FRnDef::To2D(Edge1->GetV1()->Position);
                FVector2D E2_Start = FRnDef::To2D(Edge2->GetV0()->Position);
                FVector2D E2_End = FRnDef::To2D(Edge2->GetV1()->Position);
                float T1 = 0.f;
                float T2 = 0.f;
                FVector2D Intersection;
                if (FLineUtil::SegmentIntersection(E1_Start, E1_End, E2_Start, E2_End, Intersection, T1, T2)) 
                {
                    float Z = FMath::Lerp(Edge1->GetV0()->Position.Z, Edge1->GetV1()->Position.Z, T1);
                    float Z2 = FMath::Lerp(Edge2->GetV0()->Position.Z, Edge2->GetV1()->Position.Z, T2);

                    if (FMath::Abs(Z - Z2) <= HeightTolerance) {
                        Z = (Z + Z2) * 0.5f;
                    }

                    auto NewVertex = RGraphNew<URVertex>(FVector(Intersection.X, Intersection.Y, Z));
                    Edge1->SplitEdge(NewVertex);
                    Edge2->SplitEdge(NewVertex);
                }
            }
        }
    }
}

TArray<RGraphRef_t<UREdge>> FRGraphEx::InsertVertices(RGraphRef_t<UREdge> Edge, const TArray<RGraphRef_t<URVertex>>& Vertices) {
    TArray<RGraphRef_t<UREdge>> Result;
    if (!Edge || Vertices.Num() == 0) return Result;

    Result.Add(Edge);
    for (auto Vertex : Vertices) {
        auto LastEdge = Result.Last();
        auto NewEdge = LastEdge->SplitEdge(Vertex);
        if (NewEdge) {
            Result.Add(NewEdge);
        }
    }
    return Result;
}

void FRGraphEx::SeparateFaces(RGraphRef_t<URGraph> Graph) {
    if (!Graph) return;

    auto&& Edges = Graph->GetAllEdges();
    for (auto Edge : Edges) {
        if (Edge->GetFaces().Num() > 1) {
            TArray<RGraphRef_t<URFace>> Faces = Edge->GetFaces().Array();
            for (int32 i = 1; i < Faces.Num(); ++i) {
                auto NewEdge = RGraphNew<UREdge>(Edge->GetV0(), Edge->GetV1());
                Faces[i]->ChangeEdge(Edge, NewEdge);
            }
        }
    }
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVertices(RGraphRef_t<URFace> Face) {
    TArray<RGraphRef_t<URVertex>> Result;
    if (!Face) return Result;

    TArray<RGraphRef_t<UREdge>> OutlineEdges;
    for (auto Edge : Face->GetEdges()) {
        if (Edge->GetFaces().Num() == 1) {
            OutlineEdges.Add(Edge);
        }
    }

    if (OutlineEdges.Num() > 0) {
        auto CurrentEdge = OutlineEdges[0];
        auto StartVertex = CurrentEdge->GetV0();
        auto CurrentVertex = StartVertex;

        do {
            Result.Add(CurrentVertex);
            CurrentVertex = CurrentEdge->GetOppositeVertex(CurrentVertex);

            CurrentEdge = nullptr;
            for (auto Edge : OutlineEdges) {
                if (Edge->GetV0() == CurrentVertex || Edge->GetV1() == CurrentVertex) {
                    CurrentEdge = Edge;
                    break;
                }
            }
        } while (CurrentEdge && CurrentVertex != StartVertex);
    }

    return Result;
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVertices(
    RGraphRef_t<URFaceGroup> FaceGroup,
    TFunction<bool(RGraphRef_t<URFace>)> Predicate) {
    TArray<RGraphRef_t<URVertex>> Result;
    if (!FaceGroup) return Result;

    TSet<RGraphRef_t<UREdge>> OutlineEdges;
    for (auto Face : FaceGroup->Faces) {
        if (!Predicate || Predicate(Face)) {
            for (auto Edge : Face->GetEdges()) {
                bool IsOutline = true;
                for (auto EdgeFace : Edge->GetFaces()) {
                    if (EdgeFace != Face && (!Predicate || Predicate(EdgeFace))) {
                        IsOutline = false;
                        break;
                    }
                }
                if (IsOutline) {
                    OutlineEdges.Add(Edge);
                }
            }
        }
    }

    if (OutlineEdges.Num() > 0) {
        auto CurrentEdge = *OutlineEdges.CreateIterator();
        auto StartVertex = CurrentEdge->GetV0();
        auto CurrentVertex = StartVertex;

        do {
            Result.Add(CurrentVertex);
            CurrentVertex = CurrentEdge->GetOppositeVertex(CurrentVertex);

            CurrentEdge = nullptr;
            for (auto Edge : OutlineEdges) {
                if (Edge->GetV0() == CurrentVertex || Edge->GetV1() == CurrentVertex) {
                    CurrentEdge = Edge;
                    break;
                }
            }
        } while (CurrentEdge && CurrentVertex != StartVertex);
    }

    return Result;
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVerticesByCityObjectGroup(
    RGraphRef_t<URGraph> Graph,
    UPLATEAUCityObjectGroup* CityObjectGroup,
    ERRoadTypeMask RoadTypes,
    ERRoadTypeMask RemoveRoadTypes) {
    auto Groups = GroupBy(Graph, [](RGraphRef_t<URFace> A, RGraphRef_t<URFace> B) {
        return A->GetCityObjectGroup() == B->GetCityObjectGroup();
        });

    for (auto Group : Groups) {
        if (Group->CityObjectGroup == CityObjectGroup) {
            return ComputeOutlineVertices(Group, [RoadTypes, RemoveRoadTypes](RGraphRef_t<URFace> Face) {
                return (static_cast<uint8>(Face->GetRoadTypes()) & static_cast<uint8>(RoadTypes)) != 0 &&
                    (static_cast<uint8>(Face->GetRoadTypes()) & static_cast<uint8>(RemoveRoadTypes)) == 0;
                });
        }
    }

    return TArray<RGraphRef_t<URVertex>>();
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeConvexHullVertices(RGraphRef_t<URFace> Face) {
    TArray<RGraphRef_t<URVertex>> Result;
    if (!Face) return Result;

    TArray<FVector2D> Points;
    for (auto Edge : Face->GetEdges()) {
        Points.Add(FRnDef::To2D(Edge->GetV0()->Position));
        Points.Add(FRnDef::To2D(Edge->GetV1()->Position));
    }
    TArray<RGraphRef_t<URVertex>> Vertices;
    for(auto v : CreateVertexSet(Face))
        Vertices.Add(v);
    return FGeoGraph2D::ComputeConvexVolume<RGraphRef_t<URVertex>>(
        Vertices
        , [](RGraphRef_t<URVertex> V) {return V->Position; }
        , FRnDef::Plane
        , 1e-3f);
}

// Add to implementation file
TSet<RGraphRef_t<URVertex>> FRGraphEx::CreateVertexSet(RGraphRef_t<URFace> Face)
{
    TSet<RGraphRef_t<URVertex>> Result;
    if(!Face)
        return Result;
    for (const auto Edge : Face->GetEdges()) {
        for(auto V : Edge->GetVertices())
        {
            if(V)
                Result.Add(V);
        }
    }
    return Result;
}
