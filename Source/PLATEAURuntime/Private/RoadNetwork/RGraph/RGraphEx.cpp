#include "RoadNetwork/RGraph/RGraphEx.h"

#include <array>

#include "Algo/Count.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/CityObject/SubDividedCityObject.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "Algo/AnyOf.h"
#include "RoadNetwork/Util/RnDebugEx.h"

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
    float MergeCellSizeMeter,
    int32 MergeCellLength,
    float HeightToleranceMeter) {
    if (!Graph) return TSet<RGraphRef_t<URVertex>>();

    TSet<RGraphRef_t<URVertex>> Result;
    auto&& Vertices = Graph->GetAllVertices();

    auto MergeCellSize = MergeCellSizeMeter * FRnDef::Meter2Unit;
    auto HeightTolerance = HeightToleranceMeter * FRnDef::Meter2Unit;
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
    float MergeCellSizeMeter,
    int32 MergeCellLength,
    float MidPointToleranceMeter) {
    if (!Graph) return;

    //auto&& Vertices = Graph->GetAllVertices();
    TMap<FIntVector2, TArray<RGraphRef_t<URVertex>>> Grid;

    auto MergeCellSize = MergeCellSizeMeter * FRnDef::Meter2Unit;
    auto MidPointTolerance = MidPointToleranceMeter * FRnDef::Meter2Unit;
    while(true)
    {
        auto Vertices = Graph->GetAllVertices();
        TArray<FVector> Positions;
        for (auto V : Vertices)
            Positions.Add(V->Position);
        auto Map = FGeoGraphEx::MergeVertices(Positions, MergeCellSize, MergeCellLength);
        TSet<FVector> Keys;
        for (auto& Pair : Map) {
            Keys.Add(Pair.Value);
        }

        TMap<FVector, RGraphRef_t<URVertex>> Vertex2RVertex;
        for (auto V : Keys)
            Vertex2RVertex.Add(V, RGraphNew<URVertex>(V));

        auto AfterCount = Vertex2RVertex.Num() + Algo::CountIf(Vertices, [&Vertex2RVertex](RGraphRef_t<URVertex> V) {return !Vertex2RVertex.Contains(V->Position); });
        
        for (auto V : Vertices) {
            auto Pos = V->Position;
            if (Map.Contains(Pos)) {
                auto NewVertex = Vertex2RVertex[Map[Pos]];
                V->MergeTo(NewVertex);
            }
        }

        if(AfterCount == Vertices.Num())
            break;
    }

    // a-b-cのような直線状の頂点を削除する
    while (true) 
    {
        const auto SqrLen = MidPointTolerance * MidPointTolerance;
        auto Count = 0;
        for(auto&& V : Graph->GetAllVertices())
        {
            // 2つのエッジにしか繋がっていない頂点を探す
            if (V->GetEdges().Num() != 2)
                continue;
            // 隣接する頂点が2つしかない頂点を探す
            auto Neighbors = V->GetNeighborVertices();
            if (Neighbors.Num() != 2)
                continue;
            // 中間点があってもほぼ直線だった場合は中間点は削除する
            auto segment = FLineSegment3D(Neighbors[0]->GetPosition(), Neighbors[1]->GetPosition());
            auto p = segment.GetNearestPoint(V->GetPosition());
            // とりあえずNeighbors[0]にマージする
            if ((p - V->GetPosition()).SquaredLength() < SqrLen) {
                V->MergeTo(Neighbors[0]);
                Count++;
            }
        }
        if (Count == 0)
            break;
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
    auto&& Faces = Graph->GetFaces();
    TMap<TWeakObjectPtr<UPLATEAUCityObjectGroup>, TSet<RGraphRef_t<URFace>>> Groups;
    for (auto&& Face : Faces) {
        Groups.FindOrAdd(Face->GetCityObjectGroup()).Add(Face);
    }
    for(auto&& Pair : Groups)
    {
        auto&& faces =Pair.Value;
        while (faces.IsEmpty() == false) 
        {
            TArray<RGraphRef_t<URFace>> queue;
            queue.Add(*faces.begin());
            faces.Remove(*faces.begin());
            TSet<RGraphRef_t<URFace>> g;
            while (queue.IsEmpty() == false)
            {
                RGraphRef_t<URFace> f0 = queue[0];
                queue.RemoveAt(0);
                g.Add(f0);
                for(auto&& f1 : faces) 
                {
                    if (IsShareEdge(f0, f1) && IsMatch(f0, f1)) {
                        g.Add(f1);
                        queue.Add(f1);
                        // ここではfacesから削除できない(イテレート中だから)         
                    }
                }
                for(auto&& f : queue)
                    faces.Remove(f);
            }

            Result.Add(RGraphNew<URFaceGroup>(Graph, Pair.Key.Get(), g.Array()));
        }
    }

    return Result;
}

void FRGraphEx::InsertVertexInNearEdge(RGraphRef_t<URGraph> Graph, float ToleranceMeter)
{
    if (!Graph) 
        return;
    auto Tolerance = ToleranceMeter * FRnDef::Meter2Unit;

    auto&& Vertices = Graph->GetAllVertices().Array();

    constexpr auto Comp = FRnEx::Vector3Comparer();

    auto Comparer = [&](const RGraphRef_t<URVertex>& A, const RGraphRef_t<URVertex>& B) {
        return Comp(A->Position, B->Position);
        };
    // #NOTE : UEのバグ？ ポインタのTArrayをソートしようとするとラムダ式にはポインタを消したうえで行う必要がある模様
    Vertices.Sort([&](const URVertex& A, const URVertex& B)
    {
            return Comp(A.Position, B.Position) < 0;
    });

    TSet<RGraphRef_t<UREdge>> queue;

    TMap<RGraphRef_t<UREdge>, TSet<RGraphRef_t<URVertex>>> edgeInsertMap;
    auto Threshold = Tolerance * Tolerance;

    for (auto i = 0; i < Vertices.Num(); i++) {
        auto V = Vertices[i];
        // 新規追加分
        TArray<RGraphRef_t<UREdge>> addEdges;
        TSet<RGraphRef_t<UREdge>> removeEdges;
        for(auto&& E : V->GetEdges()) 
        {
            // vと反対側の点を見る
            auto o = E->GetOppositeVertex(V);
            auto d = Comparer(V, o);
            // vが開始点の辺を追加する
            if (d < 0)
                addEdges.Add(E);
            // vが終了点の辺を取り出す
            else if (d > 0)
                removeEdges.Add(E);
        }
        for(auto&& remove : removeEdges)
            queue.Remove(remove);

        for(auto e : queue) 
        {
            if (e->GetV0() == V || e->GetV1() == V)
                continue;

            auto s = FLineSegment3D(e->GetV0()->GetPosition(), e->GetV1()->GetPosition());
            auto near = s.GetNearestPoint(V->GetPosition());
            if ((near - V->GetPosition()).SquaredLength() < Threshold) 
            {
                edgeInsertMap.FindOrAdd(e).Add(V);
            }
        }

        for(auto&& add : addEdges)
            queue.Add(add);
    }

    for(auto&& e : edgeInsertMap) {

        InsertVertices(e.Key, e.Value.Array());
    }
}

void FRGraphEx::InsertVerticesInEdgeIntersection(RGraphRef_t<URGraph> Graph, float HeightToleranceMeter) {
    if (!Graph) return;

    auto HeightTolerance = HeightToleranceMeter * FRnDef::Meter2Unit;

    auto&& Vertices = Graph->GetAllVertices().Array();

    constexpr auto Comp = FRnEx::Vector3Comparer();

    auto Comparer = [&](const RGraphRef_t<URVertex>& A, const RGraphRef_t<URVertex>& B) {
        return Comp(A->Position, B->Position);
        };
    // #NOTE : UEのバグ？ ポインタのTArrayをソートしようとするとラムダ式にはポインタを消したうえで行う必要がある模様
    Vertices.Sort([&](const URVertex& A, const URVertex& B) {
        return Comp(A.Position, B.Position) < 0;
        });

    TSet<RGraphRef_t<UREdge>> queue;

    TMap<RGraphRef_t<UREdge>, TSet<RGraphRef_t<URVertex>>> edgeInsertMap;

    TMap<FVector, RGraphRef_t<URVertex>> vertexMap;
    for (auto i = 0; i < Vertices.Num(); i++) {
        auto V = Vertices[i];
        // 新規追加分
        TArray<RGraphRef_t<UREdge>> addEdges;
        TSet<RGraphRef_t<UREdge>> removeEdges;
        for (auto&& E : V->GetEdges()) {
            // vと反対側の点を見る
            auto o = E->GetOppositeVertex(V);
            auto d = Comparer(V, o);
            // vが開始点の辺を追加する
            if (d < 0)
                addEdges.Add(E);
            // vが終了点の辺を取り出す
            else if (d > 0)
                removeEdges.Add(E);
        }

        auto NearlyEqual = [](float a, float b) {
            return FMath::Abs(a - b) < 1e-3f;
            };
        TArray<RGraphRef_t<UREdge>> targets;
        for(auto&& t : queue)
        {
            // vを端点に持つ辺は無視
            if (t->GetV0() == V || t->GetV1() == V)
                continue;
            if (removeEdges.Contains(t))
                continue;
            targets.Add(t);
        }

        for(auto e0 : removeEdges) {
            auto s0 = FLineSegment3D(e0->GetV0()->GetPosition(), e0->GetV1()->GetPosition());
            for(auto e1 : targets) {
                auto s1 = FLineSegment3D(e1->GetV0()->GetPosition(), e1->GetV1()->Position);
                // e0とe1が共有している頂点がある場合は無視
                RGraphRef_t<URVertex> shareV;
                if (e0->IsShareAnyVertex(e1, shareV)) {
                    //auto (sv, s) = s0.Magnitude < s1.Magnitude
                    //    ? (e0->GetOppositeVertex(shareV), s1)
                    //    : (e1->GetOppositeVertex(shareV), s0);
                    //if ((s.GetNearestPoint(sv.Position) - sv.Position).sqrMagnitude < shareMid)
                    //{
                    //    edgeInsertMap.GetValueOrCreate(e0).Add(sv);
                    //}

                    continue;
                }
                FVector intersection;
                float t1;
                float t2;
                if (s0.TrySegmentIntersectionBy2D(s1, FRnDef::Plane, HeightTolerance, intersection, t1, t2)) 
                {
                    // お互いの端点で交差している場合は無視
                    if ((NearlyEqual(t1, 0) || NearlyEqual(t1, 1)) && (NearlyEqual(t2, 0) || NearlyEqual(t2, 1)))
                        continue;
                    if(vertexMap.Contains(intersection) == false)
                        vertexMap.Add(intersection, RGraphNew<URVertex>(intersection));
                    auto p = vertexMap[intersection];
                    // #TODO : 0 or 1で交差した場合を考慮
                    edgeInsertMap.FindOrAdd(e0).Add(p);
                    edgeInsertMap.FindOrAdd(e1).Add(p);
                }
            }
        }

        for (auto&& add : addEdges)
            queue.Add(add);

        for (auto&& remove : removeEdges)
            queue.Remove(remove);

    }

    for (auto&& e : edgeInsertMap) {

        InsertVertices(e.Key, e.Value.Array());
    }
}

TArray<RGraphRef_t<UREdge>> FRGraphEx::InsertVertices(RGraphRef_t<UREdge> Edge, TArray<RGraphRef_t<URVertex>> Vertices)
{
    TArray<RGraphRef_t<UREdge>> Result;
    if (!Edge || Vertices.Num() == 0) return Result;

    Result.Add(Edge);

    auto O = Edge->GetV0();


    // V0 -> V1の順に並ぶようにして分割する.
    Vertices.Sort([&](const URVertex& A, const URVertex& B)
        {
            return (A.GetPosition() - O->GetPosition()).SquaredLength() < (B.GetPosition() - O->GetPosition()).SquaredLength();
        });
    for (auto Vertex : Vertices) 
    {
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

    // リストはコピーで持っておく
    auto CopiedFaces = Graph->GetFaces();
    for(auto Face : CopiedFaces)
        SeparateFace(Face);
}

void FRGraphEx::SeparateFace(RGraphRef_t<URFace> Face)
{
    // コピーする
    auto edges = Face->GetEdges();
    if (edges.IsEmpty())
        return;
    TArray<TSet<RGraphRef_t<UREdge>>> separatedEdges;
    while (edges.IsEmpty() == false) 
    {
        TQueue<RGraphRef_t<UREdge>> queue;
        queue.Enqueue(*edges.begin());
        edges.Remove(*edges.begin());

        TSet<RGraphRef_t<UREdge>> subFace;
        while (queue.IsEmpty() == false) 
        {
            RGraphRef_t<UREdge> edge;
            queue.Dequeue(edge);
            subFace.Add(edge);
            for(auto e : edge->GetNeighborEdges()) 
            {
                // すでに分離済み or 別のFaceに属している辺は無視
                if (edges.Contains(e)) {
                    // 分離すると元のリストからは削除
                    edges.Remove(e);
                    
                    queue.Enqueue(e);
                }
            }
        }
        separatedEdges.Add(subFace);
    }

    if (separatedEdges.Num() <= 1)
        return;

    // 0番目は元のFaceにする
    for (auto i = 1; i < separatedEdges.Num(); i++) 
    {
        const auto NewFace = RGraphNew<URFace>(Face->GetGraph().Get(), Face->GetCityObjectGroup().Get(), Face->GetRoadTypes(), Face->GetLodLevel());
        for (const auto E : separatedEdges[i]) {
            NewFace->AddEdge(E);
            // 元のFaceからは削除
            Face->RemoveEdge(E);
        }
    }
}


TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVertices(const TArray<RGraphRef_t<URFace>>& Faces)
{
    TSet<TObjectPtr<URVertex>> Vertices;
    TSet<TObjectPtr<UREdge>> Edges;
    for(auto&& Face : Faces)
    {
        if (!Face)
            continue;
        for(auto&& Edge : Face->GetEdges())
        {
            if (!Edge)
                continue;
            Edges.Add(Edge);
            for (auto&& V : Edge->GetVertices()) 
            {
                if (!V)
                    continue;
                Vertices.Add(V);
            }
        }
    }
    const TArray<TObjectPtr<URVertex>> VerticesArray = Vertices.Array();
    auto Result = FGeoGraph2D::ComputeOutline<TObjectPtr<URVertex>>(
        VerticesArray
        , [](const TObjectPtr<URVertex>& V) {return V->Position; }
        , FRnDef::Plane
        , [&](const TObjectPtr<URVertex>& V)
        {
            TArray<TObjectPtr<URVertex>> Res;
             for(auto&& E : V->GetEdges())
             {
                 if (Edges.Contains(E) == false)
                     continue;
                 auto Ov = E->GetOppositeVertex(V);
                 if(Ov)
                     Res.Add(Ov);
             }
             return Res;
        });

    return Result.Outline;
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVertices(RGraphRef_t<URFace> Face)
{
    const TArray Faces{ Face };
    return ComputeOutlineVertices(Faces);
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVertices(
    RGraphRef_t<URFaceGroup> FaceGroup,
    TFunction<bool(RGraphRef_t<URFace>)> Predicate) {
    TArray<RGraphRef_t<URFace>> Faces;
    for (auto&& Face : FaceGroup->Faces) {
        if (Predicate(Face))
            Faces.Add(Face);
    }
    return ComputeOutlineVertices(Faces);
}

TArray<RGraphRef_t<URVertex>> FRGraphEx::ComputeOutlineVerticesByCityObjectGroup(
    RGraphRef_t<URGraph> Graph,
    UPLATEAUCityObjectGroup* CityObjectGroup,
    ERRoadTypeMask RoadTypes,
    ERRoadTypeMask RemoveRoadTypes) {

    TArray<RGraphRef_t<URFace>> Faces;
    for (auto&& Face : Graph->GetFaces()) 
    {
        if (Face->GetCityObjectGroup() != CityObjectGroup)
            continue;
        auto FaceRoadType = Face->GetRoadTypes();
        if(!FRRoadTypeMaskEx::HasAnyFlag(FaceRoadType, RoadTypes))
            continue;
        if (FRRoadTypeMaskEx::HasAnyFlag(FaceRoadType, RemoveRoadTypes))
            continue;
            Faces.Add(Face);
    }
    return ComputeOutlineVertices(Faces);
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

bool FRGraphEx::IsShareEdge(RGraphRef_t<URFace> A, RGraphRef_t<URFace> B)
{
    if (!A || !B)
        return false;
    auto AVertices = CreateVertexSet(A);
    auto BVertices = CreateVertexSet(B);

    for (auto&& V : AVertices) {
        if (BVertices.Contains(V))
            return true;
    }

    return false;
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

void FRGraphEx::RemoveIsolatedEdgeFromFace(RGraphRef_t<URGraph> Self)
{
    for (auto&& Face : Self->GetFaces())
        RemoveIsolatedEdge(Face);

    // コピー
    auto Faces = Self->GetFaces();
    for(auto&& Face : Faces)
    {
        if(Face->GetEdges().Num() == 0)
        {
            Self->RemoveFace(Face);
        }
    }
}

TSet<RGraphRef_t<UREdge>> FRGraphEx::RemoveIsolatedEdge(RGraphRef_t<URFace> Self)
{
    auto IsIsolatedEdge = [Self](RGraphRef_t<UREdge> Edge) -> bool {
        // edgeのどっちかの頂点がedgeでしかselfに繋がっていない場合は孤立している
        return Algo::AnyOf(Edge->GetVertices(), [Self, Edge](RGraphRef_t<URVertex> V) {
            if (V == nullptr)
                return true;
            for (auto E : V->GetEdges()) {
                if (E == Edge)
                    continue;
                if (E->GetFaces().Contains(Self))
                    return false;
            }
            return true;
        });
    };
        
    TQueue<RGraphRef_t<UREdge>> removeEdgeQueue;

    for(auto&& Edge : Self->GetEdges()) 
    {
        if (IsIsolatedEdge(Edge)) {
            removeEdgeQueue.Enqueue(Edge);
        }
    }

    TSet<RGraphRef_t<UREdge>> removeEdgeSet;
    TSet<RGraphRef_t<UREdge>> disconnectedEdges;
    while (removeEdgeQueue.IsEmpty() == false) 
    {
        RGraphRef_t<UREdge> Edge;
        removeEdgeQueue.Dequeue(Edge);
        // すでに削除済みの場合は無視
        if (removeEdgeSet.Contains(Edge))
            continue;
        removeEdgeSet.Add(Edge);

        Self->RemoveEdge(Edge);

        // edgeが削除されたことにより別の辺が孤立するかもしれないのでそれも削除キューに入れる
        // 以下のような場合, b-cの辺を削除するとa-bの辺も孤立する
        // o-o
        // | |
        // o-o-a-b-c ←の様な飛び出た辺をFaceから削除する.
        for(auto E : Edge->GetNeighborEdges()) 
        {
            if (IsIsolatedEdge(E) == false)
                continue;
            removeEdgeQueue.Enqueue(E);
        }

        // どの面にも所属しない辺になったら削除する
        if (Edge->GetFaces().Num() == 0) {
            Edge->DisConnect();
            disconnectedEdges.Add(Edge);
        }
    }
    //if (removeEdgeSet.Any())
    //    DebugEx.Log($"[{self.GetDebugLabelOrDefault()}] Remove edges {removeEdgeSet.Count}");
    return disconnectedEdges;
}
