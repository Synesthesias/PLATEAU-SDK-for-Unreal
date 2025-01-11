#include "RoadNetwork/CityObject/SubDividedCityObject.h"
#include "Component/PLATEAUCityObjectGroup.h"

TArray<FSubDividedCityObject::FSubMesh> FSubDividedCityObject::FSubMesh::Separate() const{
    TArray<FSubMesh> Result;
    if (!Triangles || Triangles->Num() == 0) {
        return Result;
    }

    TArray<bool> Used;
    Used.SetNum(Triangles->Num() / 3);

    for (int32 i = 0; i < Used.Num(); ++i) {
        if (Used[i]) continue;

        FSubMesh NewMesh;
        NewMesh.Triangles = MakeShared<TArray<int32>>();

        TArray<int32> Stack = { i };
        while (Stack.Num() > 0) {
            int32 Current = Stack.Pop();
            if (Used[Current]) continue;

            Used[Current] = true;

            for (int32 j = 0; j < 3; ++j) {
                NewMesh.Triangles->Add((*Triangles)[Current * 3 + j]);
            }

            for (int32 j = 0; j < Used.Num(); ++j) {
                if (Used[j]) continue;

                bool IsConnected = false;
                for (int32 k = 0; k < 3; ++k) {
                    int32 CurrentIndex = (*Triangles)[Current * 3 + k];
                    for (int32 l = 0; l < 3; ++l) {
                        if (CurrentIndex == (*Triangles)[j * 3 + l]) {
                            IsConnected = true;
                            break;
                        }
                    }
                    if (IsConnected) break;
                }

                if (IsConnected) {
                    Stack.Add(j);
                }
            }
        }
        Result.Add(NewMesh);
    }

    return Result;
}
TArray<TArray<int32>> FSubDividedCityObject::FSubMesh::CreateOutlineIndices() const
{
    TArray<TArray<int32>> Result;
    if (!Triangles || Triangles->Num() == 0) {
        return Result;
    }

    // Create edge list
    TArray<TTuple<int32, int32>> Edges;
    for (int32 i = 0; i < Triangles->Num(); i += 3) {
        Edges.Add(MakeTuple((*Triangles)[i], (*Triangles)[i + 1]));
        Edges.Add(MakeTuple((*Triangles)[i + 1], (*Triangles)[i + 2]));
        Edges.Add(MakeTuple((*Triangles)[i + 2], (*Triangles)[i]));
    }

    // Count edge occurrences
    TMap<TTuple<int32, int32>, int32> EdgeCount;
    for (const auto& Edge : Edges) {
        auto NormalizedEdge = Edge.Get<0>() < Edge.Get<1>() ? Edge : MakeTuple(Edge.Get<1>(), Edge.Get<0>());
        EdgeCount.Add(NormalizedEdge, EdgeCount.FindRef(NormalizedEdge) + 1);
    }

    // Find outline edges
    TArray<TTuple<int32, int32>> OutlineEdges;
    for (const auto& Edge : Edges) {
        auto NormalizedEdge = Edge.Get<0>() < Edge.Get<1>() ? Edge : MakeTuple(Edge.Get<1>(), Edge.Get<0>());
        if (EdgeCount[NormalizedEdge] == 1) {
            OutlineEdges.Add(Edge);
        }
    }

    // Convert to continuous outlines
    while (OutlineEdges.Num() > 0) {
        TArray<int32> Outline;
        int32 CurrentVertex = OutlineEdges[0].Get<0>();
        Outline.Add(CurrentVertex);

        while (true) {
            bool Found = false;
            for (int32 i = 0; i < OutlineEdges.Num(); ++i) {
                if (OutlineEdges[i].Get<0>() == CurrentVertex) {
                    CurrentVertex = OutlineEdges[i].Get<1>();
                    Outline.Add(CurrentVertex);
                    OutlineEdges.RemoveAt(i);
                    Found = true;
                    break;
                }
                if (OutlineEdges[i].Get<1>() == CurrentVertex) {
                    CurrentVertex = OutlineEdges[i].Get<0>();
                    Outline.Add(CurrentVertex);
                    OutlineEdges.RemoveAt(i);
                    Found = true;
                    break;
                }
            }

            if (!Found || Outline[0] == CurrentVertex) {
                break;
            }
        }

        if (Outline.Num() >= 3) {
            Result.Add(Outline);
        }
    }

    return Result;
}

FSubDividedCityObject::FSubMesh FSubDividedCityObject::FSubMesh::DeepCopy() const {
    FSubMesh Result;
    if (Triangles) {
        Result.Triangles = MakeShared<TArray<int32>>(*Triangles);
    }
    return Result;
}

void FSubDividedCityObject::FMesh::VertexReduction() {
    if (!Vertices || Vertices->Num() == 0) {
        return;
    }

    // Create vertex mapping
    TMap<FVector, int32> UniqueVertices;
    TArray<int32> OldToNewIndices;
    OldToNewIndices.SetNum(Vertices->Num());

    TArray<FVector> NewVertices;
    int32 NewIndex = 0;

    for (int32 i = 0; i < Vertices->Num(); ++i) {
        const FVector& Vertex = (*Vertices)[i];
        int32* ExistingIndex = UniqueVertices.Find(Vertex);

        if (ExistingIndex) {
            OldToNewIndices[i] = *ExistingIndex;
        }
        else {
            UniqueVertices.Add(Vertex, NewIndex);
            OldToNewIndices[i] = NewIndex;
            NewVertices.Add(Vertex);
            NewIndex++;
        }
    }

    // Update vertices
    *Vertices = NewVertices;

    // Update indices in submeshes
    if (SubMeshes) {
        for (auto& SubMesh : *SubMeshes) {
            if (SubMesh.Triangles) {
                for (int32 i = 0; i < SubMesh.Triangles->Num(); ++i) {
                    (*SubMesh.Triangles)[i] = OldToNewIndices[(*SubMesh.Triangles)[i]];
                }
            }
        }
    }
}

void FSubDividedCityObject::FMesh::Separate() {
    if (!SubMeshes) {
        return;
    }

    TArray<FSubMesh> NewSubMeshes;
    for (const auto& SubMesh : *SubMeshes) {
        NewSubMeshes.Append(SubMesh.Separate());
    }

    *SubMeshes = NewSubMeshes;
}

FSubDividedCityObject::FMesh FSubDividedCityObject::FMesh::DeepCopy() const {
    FMesh Result;
    if (Vertices) {
        Result.Vertices = MakeShared<TArray<FVector>>(*Vertices);
    }
    if (SubMeshes) {
        Result.SubMeshes = MakeShared<TArray<FSubMesh>>();
        for (const auto& SubMesh : *SubMeshes) {
            Result.SubMeshes->Add(SubMesh.DeepCopy());
        }
    }
    return Result;
}

ERRoadTypeMask FSubDividedCityObject::GetRoadType(bool ContainsParent) const
{
    ERRoadTypeMask Result = SelfRoadType;
    if (ContainsParent) {
        Result |= ParentRoadType;
    }
    return Result;
}

FSubDividedCityObject::FSubDividedCityObject(const plateau::polygonMesh::Model& PlateauModel, TMap<FString, FPLATEAUCityObject>& CityObj)
    : bVisible(true)
    , SelfRoadType(ERRoadTypeMask::Empty)
    , ParentRoadType(ERRoadTypeMask::Empty) {

    for (int32 i = 0; i < PlateauModel.getRootNodeCount(); ++i) {
        auto& Node = PlateauModel.getRootNodeAt(i);
        FSubDividedCityObject Child(Node, CityObj, ERRoadTypeMask::Empty);
        Children.Add(Child);
    }
}

FSubDividedCityObject::FSubDividedCityObject(
    const plateau::polygonMesh::Node& PlateauNode,
    TMap<FString, FPLATEAUCityObject>& CityObj,
    ERRoadTypeMask ParentTypeMask)
    : Name(PlateauNode.getName().c_str())
    , bVisible(true)
    , SelfRoadType(ERRoadTypeMask::Empty)
    , ParentRoadType(ParentTypeMask) {

    // Convert mesh data
    if (PlateauNode.getMesh()) {
        FMesh Mesh;
        Mesh.Vertices = MakeShared<TArray<FVector>>();
        Mesh.SubMeshes = MakeShared<TArray<FSubMesh>>();

        // Convert vertices
        const auto& SrcVertices = PlateauNode.getMesh()->getVertices();
        for (int32 i = 0; i < SrcVertices.size(); ++i) 
        {
            auto&& v = SrcVertices[i];
            Mesh.Vertices->Add(FVector(v.x, v.y, v.z));
        }

        // Convert submeshes
        const auto& SrcSubMeshes = PlateauNode.getMesh()->getSubMeshes();
        const auto& indices = PlateauNode.getMesh()->getIndices();
        for (const auto& SrcSubMesh : SrcSubMeshes) {
            FSubMesh SubMesh;
            SubMesh.Triangles = MakeShared<TArray<int32>>();
            // #NOTE : PLATEAUMeshLoader.cpp参考
            for (auto Index = SrcSubMesh.getStartIndex(); Index < SrcSubMesh.getEndIndex(); Index++) 
            {                
                SubMesh.Triangles->Add(indices[Index]);
            }
            Mesh.SubMeshes->Add(SubMesh);
        }

        Meshes.Add(Mesh);
    }

    const FString DesiredName = FString(UTF8_TO_TCHAR(PlateauNode.getName().c_str()));
    if(auto Tmp = CityObj.Find(DesiredName))
        CityObject = *Tmp;

    // Process child nodes
    for (int32 i = 0; i < PlateauNode.getChildCount(); ++i) {
        auto& ChildNode = PlateauNode.getChildAt(i);
        FSubDividedCityObject Child(ChildNode, CityObj, SelfRoadType);
        Children.Add(Child);
    }
}
TArray<const FSubDividedCityObject*> FSubDividedCityObject::GetAllChildren() const{
    TArray<const FSubDividedCityObject*> Result;

    for (const auto& Child : Children) 
    {
        Result.Add(&Child);
        Result.Append(Child.GetAllChildren());
    }

    return Result;
}

void FSubDividedCityObject::SetCityObjectGroup(UPLATEAUCityObjectGroup* Group) {
    CityObjectGroup = Group;
    for (auto& Child : Children) {
        Child.SetCityObjectGroup(Group);
    }
}

TSharedPtr<FSubDividedCityObject> FSubDividedCityObject::DeepCopy()
{
    auto Result = MakeShared<FSubDividedCityObject>();
    Result->Name = Name;
    Result->SerializedCityObjects = SerializedCityObjects;
    Result->bVisible = bVisible;
    Result->CityObjectGroup = CityObjectGroup;
    Result->SelfRoadType = SelfRoadType;
    Result->ParentRoadType = ParentRoadType;

    for (const auto& Mesh : Meshes) {
        Result->Meshes.Add(Mesh.DeepCopy());
    }

    for (auto& Child : Children) {
        Result->Children.Add(*Child.DeepCopy());
    }
    return Result;
}
