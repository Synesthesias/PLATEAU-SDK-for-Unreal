#include "RoadNetwork/RGraph/RGraph.h"
FRVertex::FRVertex(const FVector& InPosition)
    : Position(InPosition) {
    Edges = MakeShared<TSet<TSharedPtr<FREdge>>>();
}

ERRoadTypeMask FRVertex::GetTypeMask() const {
    ERRoadTypeMask Result = ERRoadTypeMask::Empty;
    for (const auto& Edge : *Edges) {
        for (const auto& Face : *Edge->Faces) {
            Result = static_cast<ERRoadTypeMask>(static_cast<uint8>(Result) | static_cast<uint8>(Face->RoadTypes));
        }
    }
    return Result;
}

TArray<TSharedPtr<FRFace>> FRVertex::GetFaces() {
    TSet<TSharedPtr<FRFace>> FaceSet;
    for (const auto& Edge : *Edges) {
        for (const auto& Face : *Edge->Faces) {
            FaceSet.Add(Face);
        }
    }
    return FaceSet.Array();
}

void FRVertex::AddEdge(TSharedPtr<FREdge> Edge) {
    if (Edge) {
        Edges->Add(Edge);
    }
}

void FRVertex::RemoveEdge(TSharedPtr<FREdge> Edge) {
    if (Edge) {
        Edges->Remove(Edge);
    }
}

void FRVertex::DisConnect(bool RemoveEdge) {
    TArray<TSharedPtr<FREdge>> EdgeArray = Edges->Array();
    for (auto Edge : EdgeArray) {
        Edge->RemoveVertex(TSharedPtr<FRVertex>(this));
        if (RemoveEdge) {
            Edge->DisConnect();
        }
    }
    Edges->Empty();
}

void FRVertex::DisConnectWithKeepLink() {
    TArray<TSharedPtr<FREdge>> EdgeArray = Edges->Array();
    for (auto Edge : EdgeArray) {
        Edge->RemoveVertex(TSharedPtr<FRVertex>(this));
    }
    Edges->Empty();
}

TArray<TSharedPtr<FRVertex>> FRVertex::GetNeighborVertices() {
    TArray<TSharedPtr<FRVertex>> Result;
    for (auto Edge : *Edges) {
        Result.Add(Edge->GetOppositeVertex(TSharedPtr<FRVertex>(this)));
    }
    return Result;
}

bool FRVertex::IsNeighbor(TSharedPtr<FRVertex> Other) {
    if (!Other) return false;

    for (auto Edge : *Edges) {
        if (Edge->GetOppositeVertex(TSharedPtr<FRVertex>(this)) == Other) {
            return true;
        }
    }
    return false;
}

void FRVertex::MergeTo(TSharedPtr<FRVertex> Dst, bool CheckEdgeMerge) {
    if (!Dst) return;

    TArray<TSharedPtr<FREdge>> EdgeArray = Edges->Array();
    for (auto Edge : EdgeArray) {
        Edge->ChangeVertex(TSharedPtr<FRVertex>(this), Dst);
        if (CheckEdgeMerge) {
            for (auto DstEdge : *Dst->Edges) {
                if (Edge != DstEdge && Edge->IsSameVertex(DstEdge)) {
                    Edge->MergeTo(DstEdge);
                    break;
                }
            }
        }
    }
}

ERRoadTypeMask FRVertex::GetRoadType(TFunction<bool(TSharedPtr<FRFace>)> FaceSelector) {
    ERRoadTypeMask Result = ERRoadTypeMask::Empty;
    for (auto Edge : *Edges) {
        for (auto Face : *Edge->Faces) {
            if (!FaceSelector || FaceSelector(Face)) {
                Result = static_cast<ERRoadTypeMask>(static_cast<uint8>(Result) | static_cast<uint8>(Face->RoadTypes));
            }
        }
    }
    return Result;
}

int32 FRVertex::GetMaxLodLevel(TFunction<bool(TSharedPtr<FRFace>)> FaceSelector) {
    int32 MaxLevel = -1;
    for (auto Edge : *Edges) {
        for (auto Face : *Edge->Faces) {
            if (!FaceSelector || FaceSelector(Face)) {
                MaxLevel = FMath::Max(MaxLevel, Face->LodLevel);
            }
        }
    }
    return MaxLevel;
}

FREdge::FREdge(TSharedPtr<FRVertex> InV0, TSharedPtr<FRVertex> InV1) {
    Faces = MakeShared<TSet<TSharedPtr<FRFace>>>();
    Vertices.Add(InV0);
    Vertices.Add(InV1);

    if (InV0) InV0->AddEdge(TSharedPtr<FREdge>(this));
    if (InV1) InV1->AddEdge(TSharedPtr<FREdge>(this));
}

bool FREdge::IsValid() const {
    return Vertices[0] != nullptr && Vertices[1] != nullptr;
}

TArray<TSharedPtr<FREdge>> FREdge::GetNeighborEdges() {
    TSet<TSharedPtr<FREdge>> Result;
    for (auto Vertex : Vertices) {
        if (Vertex) {
            Result.Append(*Vertex->Edges);
        }
    }
    Result.Remove(TSharedPtr<FREdge>(this));
    return Result.Array();
}

TSharedPtr<FRVertex> FREdge::GetVertex(EVertexType Type) {
    return Vertices[static_cast<int32>(Type)];
}

void FREdge::SetVertex(EVertexType Type, TSharedPtr<FRVertex> Vertex) {
    auto OldVertex = Vertices[static_cast<int32>(Type)];
    if (OldVertex) {
        OldVertex->RemoveEdge(TSharedPtr<FREdge>(this));
    }

    Vertices[static_cast<int32>(Type)] = Vertex;
    if (Vertex) {
        Vertex->AddEdge(TSharedPtr<FREdge>(this));
    }
}

void FREdge::ChangeVertex(TSharedPtr<FRVertex> From, TSharedPtr<FRVertex> To) {
    if (Vertices[0] == From) {
        SetVertex(EVertexType::V0, To);
    }
    else if (Vertices[1] == From) {
        SetVertex(EVertexType::V1, To);
    }
}

void FREdge::AddFace(TSharedPtr<FRFace> Face) {
    if (Face) {
        Faces->Add(Face);
    }
}

void FREdge::RemoveFace(TSharedPtr<FRFace> Face) {
    if (Face) {
        Faces->Remove(Face);
    }
}

void FREdge::RemoveVertex(TSharedPtr<FRVertex> Vertex) {
    if (Vertices[0] == Vertex) {
        SetVertex(EVertexType::V0, nullptr);
    }
    else if (Vertices[1] == Vertex) {
        SetVertex(EVertexType::V1, nullptr);
    }
}

void FREdge::DisConnect() {
    TArray<TSharedPtr<FRFace>> FaceArray = Faces->Array();
    for (auto Face : FaceArray) {
        Face->RemoveEdge(TSharedPtr<FREdge>(this));
    }
    Faces->Empty();

    for (auto Vertex : Vertices) {
        if (Vertex) {
            Vertex->RemoveEdge(TSharedPtr<FREdge>(this));
        }
    }
}

TSharedPtr<FREdge> FREdge::SplitEdge(TSharedPtr<FRVertex> V) {
    if (!V) return nullptr;

    auto NewEdge = MakeShared<FREdge>(V, Vertices[1]);
    SetVertex(EVertexType::V1, V);

    TArray<TSharedPtr<FRFace>> FaceArray = Faces->Array();
    for (auto Face : FaceArray) {
        Face->AddEdge(NewEdge);
    }

    return NewEdge;
}

bool FREdge::IsSameVertex(TSharedPtr<FRVertex> V0, TSharedPtr<FRVertex> V1) {
    return (Vertices[0] == V0 && Vertices[1] == V1) ||
        (Vertices[0] == V1 && Vertices[1] == V0);
}

bool FREdge::IsSameVertex(TSharedPtr<FREdge> Other) {
    if (!Other) return false;
    return IsSameVertex(Other->Vertices[0], Other->Vertices[1]);
}

bool FREdge::IsShareAnyVertex(TSharedPtr<FREdge> Other) {
    if (!Other) return false;
    return Vertices[0] == Other->Vertices[0] ||
        Vertices[0] == Other->Vertices[1] ||
        Vertices[1] == Other->Vertices[0] ||
        Vertices[1] == Other->Vertices[1];
}

bool FREdge::IsShareAnyVertex(TSharedPtr<FREdge> Other, TSharedPtr<FRVertex>& SharedVertex) {
    if (!Other) return false;

    if (Vertices[0] == Other->Vertices[0] || Vertices[0] == Other->Vertices[1]) {
        SharedVertex = Vertices[0];
        return true;
    }
    if (Vertices[1] == Other->Vertices[0] || Vertices[1] == Other->Vertices[1]) {
        SharedVertex = Vertices[1];
        return true;
    }
    return false;
}

bool FREdge::TryGetOppositeVertex(TSharedPtr<FRVertex> Vertex, TSharedPtr<FRVertex>& Opposite) {
    if (Vertices[0] == Vertex) {
        Opposite = Vertices[1];
        return true;
    }
    if (Vertices[1] == Vertex) {
        Opposite = Vertices[0];
        return true;
    }
    return false;
}

TSharedPtr<FRVertex> FREdge::GetOppositeVertex(TSharedPtr<FRVertex> Vertex) {
    return (Vertices[0] == Vertex) ? Vertices[1] : Vertices[0];
}

void FREdge::MergeTo(TSharedPtr<FREdge> Dst, bool CheckFaceMerge) {
    if (!Dst) return;

    TArray<TSharedPtr<FRFace>> FaceArray = Faces->Array();
    for (auto Face : FaceArray) {
        Face->ChangeEdge(TSharedPtr<FREdge>(this), Dst);
        if (CheckFaceMerge) {
            for (auto DstFace : *Dst->Faces) {
                if (Face != DstFace && Face->IsSameEdges(DstFace)) {
                    Face->TryMergeTo(DstFace);
                    break;
                }
            }
        }
    }
}

// Face implementations
FRFace::FRFace(TSharedPtr<FRGraph> InGraph, UPLATEAUCityObjectGroup* InCityObjectGroup, ERRoadTypeMask InRoadType, int32 InLodLevel)
    : bVisible(true)
    , CityObjectGroup(InCityObjectGroup)
    , RoadTypes(InRoadType)
    , LodLevel(InLodLevel)
    , Graph(InGraph) {
    Edges = MakeShared<TSet<TSharedPtr<FREdge>>>();
    if (Graph) {
        Graph->AddFace(TSharedPtr<FRFace>(this));
    }
}

void FRFace::AddEdge(TSharedPtr<FREdge> Edge) {
    if (Edge) {
        Edges->Add(Edge);
        Edge->AddFace(TSharedPtr<FRFace>(this));
    }
}

void FRFace::RemoveGraph(TSharedPtr<FRGraph> G) {
    if (Graph == G) {
        Graph = nullptr;
    }
}

void FRFace::SetGraph(TSharedPtr<FRGraph> G) {
    if (Graph != G) {
        if (Graph) {
            Graph->RemoveFace(TSharedPtr<FRFace>(this));
        }
        Graph = G;
        if (Graph) {
            Graph->AddFace(TSharedPtr<FRFace>(this));
        }
    }
}

void FRFace::RemoveEdge(TSharedPtr<FREdge> Edge) {
    if (Edge) {
        Edges->Remove(Edge);
        Edge->RemoveFace(TSharedPtr<FRFace>(this));
    }
}

void FRFace::ChangeEdge(TSharedPtr<FREdge> From, TSharedPtr<FREdge> To) {
    if (From && To && From != To) {
        RemoveEdge(From);
        AddEdge(To);
    }
}

bool FRFace::IsSameEdges(TSharedPtr<FRFace> Other) {
    if (!Other || Edges->Num() != Other->Edges->Num()) {
        return false;
    }

    for (auto Edge : *Edges) {
        if (!Other->Edges->Contains(Edge)) {
            return false;
        }
    }
    return true;
}

bool FRFace::TryMergeTo(TSharedPtr<FRFace> Dst) {
    if (!Dst || !IsSameEdges(Dst)) {
        return false;
    }

    DisConnect();
    return true;
}

void FRFace::DisConnect() {
    TArray<TSharedPtr<FREdge>> EdgeArray = Edges->Array();
    for (auto Edge : EdgeArray) {
        RemoveEdge(Edge);
    }
    SetGraph(nullptr);
}

TArray<TSharedPtr<FREdge>> FRGraph::GetAllEdges() {
    TSet<TSharedPtr<FREdge>> Result;
    for (auto Face : *Faces) {
        Result.Append(*Face->Edges);
    }
    return Result.Array();
}

TArray<TSharedPtr<FRVertex>> FRGraph::GetAllVertices() {
    TSet<TSharedPtr<FRVertex>> Result;
    for (auto Edge : GetAllEdges()) {
        Result.Add(Edge->GetV0());
        Result.Add(Edge->GetV1());
    }
    return Result.Array();
}

void FRGraph::AddFace(TSharedPtr<FRFace> Face) {
    if (Face) {
        Faces->Add(Face);
    }
}

void FRGraph::RemoveFace(TSharedPtr<FRFace> Face) {
    if (Face) {
        Faces->Remove(Face);
    }
}

// Face Group implementations
FRFaceGroup::FRFaceGroup(TSharedPtr<FRGraph> InGraph, UPLATEAUCityObjectGroup* InCityObjectGroup, const TArray<TSharedPtr<FRFace>>& InFaces)
    : Graph(InGraph)
    , CityObjectGroup(InCityObjectGroup) {
    Faces = MakeShared<TSet<TSharedPtr<FRFace>>>();
    for (auto Face : InFaces) {
        Faces->Add(Face);
    }
}

ERRoadTypeMask FRFaceGroup::GetRoadTypes() const {
    ERRoadTypeMask Result = ERRoadTypeMask::Empty;
    for (auto Face : *Faces) {
        Result = static_cast<ERRoadTypeMask>(static_cast<uint8>(Result) | static_cast<uint8>(Face->RoadTypes));
    }
    return Result;
}
