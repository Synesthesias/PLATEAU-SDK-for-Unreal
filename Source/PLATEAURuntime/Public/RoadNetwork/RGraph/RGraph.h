#pragma once
// ☀
#pragma once

#include "CoreMinimal.h"
#include "RGraphDef.h"
#include <memory>

class UPLATEAUCityObjectGroup;

class FRVertex;
class FREdge;
class FRFace;
class FRGraph;

class FRVertex {
public:
    FRVertex(const FVector& InPosition);

    TSharedPtr<TSet<TSharedPtr<FREdge>>> Edges;
    FVector Position;

    const TSet<TSharedPtr<FREdge>>& GetEdges() const { return *Edges; }
    ERRoadTypeMask GetTypeMask() const;
    TArray<TSharedPtr<FRFace>> GetFaces();
    void AddEdge(TSharedPtr<FREdge> Edge);
    void RemoveEdge(TSharedPtr<FREdge> Edge);
    void DisConnect(bool RemoveEdge = false);
    void DisConnectWithKeepLink();
    TArray<TSharedPtr<FRVertex>> GetNeighborVertices();
    bool IsNeighbor(TSharedPtr<FRVertex> Other);
    void MergeTo(TSharedPtr<FRVertex> Dst, bool CheckEdgeMerge = true);

    ERRoadTypeMask GetRoadType(TFunction<bool(TSharedPtr<FRFace>)> FaceSelector = nullptr);
    int32 GetMaxLodLevel(TFunction<bool(TSharedPtr<FRFace>)> FaceSelector = nullptr);
};

class FREdge {
public:
    enum class EVertexType : uint8 {
        V0,
        V1
    };

    FREdge(TSharedPtr<FRVertex> InV0, TSharedPtr<FRVertex> InV1);

    TSharedPtr<TSet<TSharedPtr<FRFace>>> Faces;
    TArray<TSharedPtr<FRVertex>> Vertices;

    const TSet<TSharedPtr<FRFace>>& GetFaces() const { return *Faces; }
    TSharedPtr<FRVertex> GetV0() const { return Vertices[0]; }
    TSharedPtr<FRVertex> GetV1() const { return Vertices[1]; }
    const TArray<TSharedPtr<FRVertex>>& GetVertices() const { return Vertices; }
    bool IsValid() const;
    TArray<TSharedPtr<FREdge>> GetNeighborEdges();
    TSharedPtr<FRVertex> GetVertex(EVertexType Type);
    void SetVertex(EVertexType Type, TSharedPtr<FRVertex> Vertex);
    void ChangeVertex(TSharedPtr<FRVertex> From, TSharedPtr<FRVertex> To);
    void AddFace(TSharedPtr<FRFace> Face);
    void RemoveFace(TSharedPtr<FRFace> Face);
    void RemoveVertex(TSharedPtr<FRVertex> Vertex);
    void DisConnect();
    TSharedPtr<FREdge> SplitEdge(TSharedPtr<FRVertex> V);
    bool IsSameVertex(TSharedPtr<FRVertex> V0, TSharedPtr<FRVertex> V1);
    bool IsSameVertex(TSharedPtr<FREdge> Other);
    bool IsShareAnyVertex(TSharedPtr<FREdge> Other);
    bool IsShareAnyVertex(TSharedPtr<FREdge> Other, TSharedPtr<FRVertex>& SharedVertex);
    bool TryGetOppositeVertex(TSharedPtr<FRVertex> Vertex, TSharedPtr<FRVertex>& Opposite);
    TSharedPtr<FRVertex> GetOppositeVertex(TSharedPtr<FRVertex> Vertex);
    void MergeTo(TSharedPtr<FREdge> Dst, bool CheckFaceMerge = true);
};

class FRFace {
public:
    FRFace(TSharedPtr<FRGraph> InGraph, UPLATEAUCityObjectGroup* InCityObjectGroup, ERRoadTypeMask InRoadType, int32 InLodLevel);

    bool bVisible;
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
    ERRoadTypeMask RoadTypes;
    int32 LodLevel;
    TSharedPtr<FRGraph> Graph;
    TSharedPtr<TSet<TSharedPtr<FREdge>>> Edges;

    const TSet<TSharedPtr<FREdge>>& GetEdges() const { return *Edges; }
    bool IsValid() const { return Edges->Num() > 0; }
    void AddEdge(TSharedPtr<FREdge> Edge);
    void RemoveGraph(TSharedPtr<FRGraph> G);
    void SetGraph(TSharedPtr<FRGraph> G);
    void RemoveEdge(TSharedPtr<FREdge> Edge);
    void ChangeEdge(TSharedPtr<FREdge> From, TSharedPtr<FREdge> To);
    bool IsSameEdges(TSharedPtr<FRFace> Other);
    bool TryMergeTo(TSharedPtr<FRFace> Dst);
    void DisConnect();
};

class FRGraph {
public:
    TSharedPtr<TSet<TSharedPtr<FRFace>>> Faces;

    const TSet<TSharedPtr<FRFace>>& GetFaces() const { return *Faces; }
    TArray<TSharedPtr<FREdge>> GetAllEdges();
    TArray<TSharedPtr<FRVertex>> GetAllVertices();
    void AddFace(TSharedPtr<FRFace> Face);
    void RemoveFace(TSharedPtr<FRFace> Face);
};

class FRFaceGroup {
public:
    FRFaceGroup(TSharedPtr<FRGraph> InGraph, UPLATEAUCityObjectGroup* InCityObjectGroup, const TArray<TSharedPtr<FRFace>>& InFaces);

    TSharedPtr<FRGraph> Graph;
    TWeakObjectPtr<UPLATEAUCityObjectGroup> CityObjectGroup;
    TSharedPtr<TSet<TSharedPtr<FRFace>>> Faces;

    ERRoadTypeMask GetRoadTypes() const;
};
