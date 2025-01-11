#include "RoadNetwork/RGraph/RGraphEx.h"

#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/CityObject/SubDividedCityObject.h"

namespace
{
    struct FEdgeKey
    {
        TSharedPtr<FRVertex> V0;
        TSharedPtr<FRVertex> V1;

        FEdgeKey(TSharedPtr<FRVertex> InV0, TSharedPtr<FRVertex> InV1)
            : V0(InV0), V1(InV1) {
        }

        bool operator==(const FEdgeKey& Other) const {
            return (V0 == Other.V0 && V1 == Other.V1) || (V0 == Other.V1 && V1 == Other.V0);
        }

        friend uint32 GetTypeHash(const FEdgeKey& Key) {
            return HashCombine(GetTypeHash(Key.V0), GetTypeHash(Key.V1));
        }
    };
}

void FRGraphHelper::RemoveInnerVertex(TSharedPtr<FRFace> Face) {
    if (!Face) return;

    TArray<TSharedPtr<FRVertex>> Vertices;
    for (auto Edge : *Face->Edges) {
        Vertices.Add(Edge->GetV0());
        Vertices.Add(Edge->GetV1());
    }

    for (auto Vertex : Vertices) {
        if (Vertex->GetFaces().Num() == 1) {
            Vertex->DisConnect(true);
        }
    }
}

void FRGraphHelper::RemoveInnerVertex(TSharedPtr<FRGraph> Graph) {
    if (!Graph) return;

    for (auto Face : *Graph->Faces) {
        RemoveInnerVertex(Face);
    }
}

TSet<TSharedPtr<FRVertex>> FRGraphHelper::AdjustSmallLodHeight(
    TSharedPtr<FRGraph> Graph,
    float MergeCellSize,
    int32 MergeCellLength,
    float HeightTolerance) {
    if (!Graph) return TSet<TSharedPtr<FRVertex>>();

    TSet<TSharedPtr<FRVertex>> Result;
    TArray<TSharedPtr<FRVertex>> Vertices = Graph->GetAllVertices();

    // Create grid for vertex grouping
    TMap<FIntVector2, TArray<TSharedPtr<FRVertex>>> Grid;
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
        TMap<int32, TArray<TSharedPtr<FRVertex>>> LodGroups;
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

void FRGraphHelper::VertexReduction(
    TSharedPtr<FRGraph> Graph,
    float MergeCellSize,
    int32 MergeCellLength,
    float MidPointTolerance) {
    if (!Graph) return;

    TArray<TSharedPtr<FRVertex>> Vertices = Graph->GetAllVertices();
    TMap<FIntVector2, TArray<TSharedPtr<FRVertex>>> Grid;

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
        TMap<TTuple<ERRoadTypeMask, int32>, TArray<TSharedPtr<FRVertex>>> TypeLodGroups;
        for (auto Vertex : CellVertices) {
            ERRoadTypeMask RoadType = Vertex->GetTypeMask();
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
            TSharedPtr<FRVertex> ClosestVertex = nullptr;
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

void FRGraphHelper::EdgeReduction(TSharedPtr<FRGraph> Graph) {
    if (!Graph) return;

    TArray<TSharedPtr<FREdge>> Edges = Graph->GetAllEdges();
    for (auto Edge : Edges) {
        for (auto OtherEdge : Edge->GetNeighborEdges()) {
            if (Edge->IsSameVertex(OtherEdge)) {
                Edge->MergeTo(OtherEdge);
                break;
            }
        }
    }
}

void FRGraphHelper::MergeIsolatedVertices(TSharedPtr<FRGraph> Graph) {
    if (!Graph) return;

    for (auto Face : *Graph->Faces) {
        MergeIsolatedVertex(Face);
    }
}

void FRGraphHelper::MergeIsolatedVertex(TSharedPtr<FRFace> Face) {
    if (!Face) return;

    TArray<TSharedPtr<FRVertex>> Vertices;
    for (auto Edge : *Face->Edges) {
        Vertices.Add(Edge->GetV0());
        Vertices.Add(Edge->GetV1());
    }

    for (auto Vertex : Vertices) {
        if (Vertex->GetEdges().Num() == 2) {
            TArray<TSharedPtr<FREdge>> VertexEdges = Vertex->GetEdges().Array();
            if (VertexEdges.Num() == 2) {
                TSharedPtr<FRVertex> V0 = VertexEdges[0]->GetOppositeVertex(Vertex);
                TSharedPtr<FRVertex> V1 = VertexEdges[1]->GetOppositeVertex(Vertex);

                if (V0 && V1 && !V0->IsNeighbor(V1)) {
                    auto NewEdge = MakeShared<FREdge>(V0, V1);
                    for (auto Edge : VertexEdges) {
                        for (auto EdgeFace : *Edge->Faces) {
                            EdgeFace->AddEdge(NewEdge);
                        }
                    }
                    Vertex->DisConnect(true);
                }
            }
        }
    }
}

TArray<TSharedPtr<FRFaceGroup>> FRGraphHelper::GroupBy(
    TSharedPtr<FRGraph> Graph,
    TFunction<bool(TSharedPtr<FRFace>, TSharedPtr<FRFace>)> IsMatch) {
    TArray<TSharedPtr<FRFaceGroup>> Result;
    if (!Graph) 
        return Result;

    TSet<TSharedPtr<FRFace>> UnprocessedFaces(*Graph->Faces);
    while (UnprocessedFaces.Num() > 0) {
        TArray<TSharedPtr<FRFace>> GroupFaces;
        auto FirstFace = *UnprocessedFaces.CreateIterator();
        GroupFaces.Add(FirstFace);
        UnprocessedFaces.Remove(FirstFace);

        for (int32 i = 0; i < GroupFaces.Num(); ++i) {
            auto Face = GroupFaces[i];
            TArray<TSharedPtr<FRFace>> Neighbors;
            for (auto Edge : *Face->Edges) {
                Neighbors.Append(Edge->GetFaces().Array());
            }

            for (auto Neighbor : Neighbors) {
                if (UnprocessedFaces.Contains(Neighbor) && IsMatch(Face, Neighbor)) {
                    GroupFaces.Add(Neighbor);
                    UnprocessedFaces.Remove(Neighbor);
                }
            }
        }

        Result.Add(MakeShared<FRFaceGroup>(Graph, FirstFace->CityObjectGroup.Get(), GroupFaces));
    }

    return Result;
}

void FRGraphHelper::Optimize(
    TSharedPtr<FRGraph> Graph,
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

void FRGraphHelper::InsertVertexInNearEdge(TSharedPtr<FRGraph> Graph, float Tolerance)
{
    if (!Graph) return;

    TArray<TSharedPtr<FRVertex>> Vertices = Graph->GetAllVertices();
    TArray<TSharedPtr<FREdge>> Edges = Graph->GetAllEdges();
    TArray<TTuple<TSharedPtr<FRVertex>, TSharedPtr<FREdge>>> InsertVertices;

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

void FRGraphHelper::InsertVerticesInEdgeIntersection(TSharedPtr<FRGraph> Graph, float HeightTolerance) {
    if (!Graph) return;

    TArray<TSharedPtr<FREdge>> Edges = Graph->GetAllEdges();
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

                    auto NewVertex = MakeShared<FRVertex>(FVector(Intersection.X, Intersection.Y, Z));
                    Edge1->SplitEdge(NewVertex);
                    Edge2->SplitEdge(NewVertex);
                }
            }
        }
    }
}

TArray<TSharedPtr<FREdge>> FRGraphHelper::InsertVertices(TSharedPtr<FREdge> Edge, const TArray<TSharedPtr<FRVertex>>& Vertices) {
    TArray<TSharedPtr<FREdge>> Result;
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

void FRGraphHelper::SeparateFaces(TSharedPtr<FRGraph> Graph) {
    if (!Graph) return;

    TArray<TSharedPtr<FREdge>> Edges = Graph->GetAllEdges();
    for (auto Edge : Edges) {
        if (Edge->GetFaces().Num() > 1) {
            TArray<TSharedPtr<FRFace>> Faces = Edge->GetFaces().Array();
            for (int32 i = 1; i < Faces.Num(); ++i) {
                auto NewEdge = MakeShared<FREdge>(Edge->GetV0(), Edge->GetV1());
                Faces[i]->ChangeEdge(Edge, NewEdge);
            }
        }
    }
}

TArray<TSharedPtr<FRVertex>> FRGraphHelper::ComputeOutlineVertices(TSharedPtr<FRFace> Face) {
    TArray<TSharedPtr<FRVertex>> Result;
    if (!Face) return Result;

    TArray<TSharedPtr<FREdge>> OutlineEdges;
    for (auto Edge : *Face->Edges) {
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

TArray<TSharedPtr<FRVertex>> FRGraphHelper::ComputeOutlineVertices(
    TSharedPtr<FRFaceGroup> FaceGroup,
    TFunction<bool(TSharedPtr<FRFace>)> Predicate) {
    TArray<TSharedPtr<FRVertex>> Result;
    if (!FaceGroup) return Result;

    TSet<TSharedPtr<FREdge>> OutlineEdges;
    for (auto Face : *FaceGroup->Faces) {
        if (!Predicate || Predicate(Face)) {
            for (auto Edge : *Face->Edges) {
                bool IsOutline = true;
                for (auto EdgeFace : *Edge->Faces) {
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

TArray<TSharedPtr<FRVertex>> FRGraphHelper::ComputeOutlineVerticesByCityObjectGroup(
    TSharedPtr<FRGraph> Graph,
    UPLATEAUCityObjectGroup* CityObjectGroup,
    ERRoadTypeMask RoadTypes,
    ERRoadTypeMask RemoveRoadTypes) {
    auto Groups = GroupBy(Graph, [](TSharedPtr<FRFace> A, TSharedPtr<FRFace> B) {
        return A->CityObjectGroup == B->CityObjectGroup;
        });

    for (auto Group : Groups) {
        if (Group->CityObjectGroup == CityObjectGroup) {
            return ComputeOutlineVertices(Group, [RoadTypes, RemoveRoadTypes](TSharedPtr<FRFace> Face) {
                return (static_cast<uint8>(Face->RoadTypes) & static_cast<uint8>(RoadTypes)) != 0 &&
                    (static_cast<uint8>(Face->RoadTypes) & static_cast<uint8>(RemoveRoadTypes)) == 0;
                });
        }
    }

    return TArray<TSharedPtr<FRVertex>>();
}

TArray<TSharedPtr<FRVertex>> FRGraphHelper::ComputeConvexHullVertices(TSharedPtr<FRFace> Face) {
    TArray<TSharedPtr<FRVertex>> Result;
    if (!Face) return Result;

    TArray<FVector2D> Points;
    for (auto Edge : *Face->Edges) {
        Points.Add(FRnDef::To2D(Edge->GetV0()->Position));
        Points.Add(FRnDef::To2D(Edge->GetV1()->Position));
    }
    TArray<TSharedPtr<FRVertex>> Vertices;
    for(auto v : CreateVertexSet(Face))
        Vertices.Add(v);
    return FGeoGraph2D::ComputeConvexVolume<TSharedPtr<FRVertex>>(
        Vertices
        , [](TSharedPtr<FRVertex> V) {return V->Position; }
        , FRnDef::Plane
        , 1e-3f);
}

// Add to implementation file
TSet<TSharedPtr<FRVertex>> FRGraphHelper::CreateVertexSet(TSharedPtr<FRFace> Face)
{
    TSet<TSharedPtr<FRVertex>> Result;
    if(!Face)
        return Result;
    for (const auto Edge : *(Face->Edges)) {
        for(auto V : Edge->GetVertices())
        {
            if(V)
                Result.Add(V);
        }
    }
    return Result;
}

TSharedPtr<FRGraph> FRGraphHelper::CreateGraph(const TArray<TSharedPtr<FSubDividedCityObject>>& CityObjects, bool useOutline)
{
    auto Graph = MakeShared<FRGraph>();

    TMap<FVector, TSharedPtr<FRVertex>> vertexMap;
    TMap<FEdgeKey, TSharedPtr<FREdge>> edgeMap;
    for(auto CityObject : CityObjects) {
        if (CityObject->CityObjectGroup == nullptr) {
            continue;
        }

        auto&& lodLevel = CityObject->CityObjectGroup->MinLOD;
        auto&& roadType = CityObject->GetRoadType(true);
        // transformを適用する
        auto&& mat = CityObject->CityObjectGroup->GetRelativeTransform().ToMatrixWithScale();
        for(auto&& mesh : CityObject->Meshes) {
            auto&& face = MakeShared<FRFace>(Graph, CityObject->CityObjectGroup.Get(), roadType, lodLevel);

            TArray<TSharedPtr<FRVertex>> vertices;
            for(auto&& v : mesh.Vertices)
            {
                // #TODO : RN
                auto v4 = FVector4(v, 1.f);// *mat;

                if(vertexMap.Contains(v4) == false)
                {
                    vertexMap.Add(v4, MakeShared<FRVertex>(v4));
                }

                vertices.Add(vertexMap[v4]);
            }
            for(auto&& s : mesh.SubMeshes) {
                auto AddEdge = [&edgeMap, &face](TSharedPtr<FRVertex> V0, TSharedPtr<FRVertex> V1) {

                    auto key = FEdgeKey(V0, V1);
                    if (edgeMap.Contains(key) == false)
                        edgeMap[key] = MakeShared<FREdge>(key.V0, key.V1);
                    face->AddEdge(edgeMap[key]);
                };
                if (useOutline) {
                    auto&& indexTable = s.CreateOutlineIndices();
                    for(auto&& indices : indexTable) {
                        for (auto&& i = 0; i < indices.Num(); i++) 
                        {
                            AddEdge(vertices[indices[i]], vertices[indices[(i + 1) % indices.Num()]]);
                        }
                    }
                }
                else {
                    for (auto&& i = 0; i < s.Triangles->Num(); i += 3) 
                    {
                        AddEdge(vertices[(*s.Triangles)[i + 0]], vertices[(*s.Triangles)[i + 1]]);
                        AddEdge(vertices[(*s.Triangles)[i + 1]], vertices[(*s.Triangles)[i + 2]]);
                        AddEdge(vertices[(*s.Triangles)[i + 2]], vertices[(*s.Triangles)[i]]);
                    }
                }

            }
            Graph->AddFace(face);
        }
    }
    return Graph;
}
