#include "RoadNetwork/CityObject/SubDividedCityObject.h"
#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/Util/RnDebugEx.h"

TArray<FSubDividedCityObjectSubMesh> FSubDividedCityObjectSubMesh::Separate() const{
    TArray<FSubDividedCityObjectSubMesh> Result;
    if (Triangles.Num() == 0) {
        return Result;
    }

    TArray<bool> Used;
    Used.SetNum(Triangles.Num() / 3);

    for (int32 i = 0; i < Used.Num(); ++i) {
        if (Used[i]) continue;

        FSubDividedCityObjectSubMesh NewMesh;
        TArray<int32> Stack = { i };
        while (Stack.Num() > 0) {
            int32 Current = Stack.Pop();
            if (Used[Current]) continue;

            Used[Current] = true;

            for (int32 j = 0; j < 3; ++j) {
                NewMesh.Triangles.Add(Triangles[Current * 3 + j]);
            }

            for (int32 j = 0; j < Used.Num(); ++j) {
                if (Used[j]) continue;

                bool IsConnected = false;
                for (int32 k = 0; k < 3; ++k) {
                    int32 CurrentIndex = Triangles[Current * 3 + k];
                    for (int32 l = 0; l < 3; ++l) {
                        if (CurrentIndex == Triangles[j * 3 + l]) {
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
TArray<TArray<int32>> FSubDividedCityObjectSubMesh::CreateOutlineIndices() const
{
    TArray<TArray<int32>> Result;
    if (Triangles.Num() == 0) {
        return Result;
    }

    // Create edge list
    TArray<TTuple<int32, int32>> Edges;

    auto GetEdge = [](int32 A, int32 B)
    {
        if (A < B)
            Swap(A, B);
        return MakeTuple(A, B);
    };

    // Count edge occurrences
    // Key   : 辺
    // Value : 0以上の場合は三角形のインデックス、-1の場合は複数の三角形に接続している
    TMap<TTuple<int32, int32>, int32> Edge2Triangle;
    for (int32 i = 0; i < Triangles.Num(); i += 3) 
    {
        auto T = i / 3;
        for(auto X = 0; X < 3; ++X)
        {
            auto A = Triangles[i + X];
            auto B = Triangles[i + (X + 1) % 3];
            auto E = GetEdge(A, B);
            if (Edge2Triangle.Contains(E) == false) {
                Edge2Triangle.Add(E, T);
            }
            else if (Edge2Triangle[E] != T)
            {
                Edge2Triangle[E] = -1;                
            }
        }
    }
    TArray< TTuple<int32, int32>> OutlineEdges;
    for(auto& E : Edge2Triangle)
    {
        if(E.Value >= 0)
        {
            OutlineEdges.Add(E.Key);
        }
    }

    // Convert to continuous outlines
    while (OutlineEdges.Num() > 0) 
    {
        TArray<int32> Outline;
        auto Edge = OutlineEdges[0];
        OutlineEdges.RemoveAt(0);

        auto Indices = TArray<int32>{ Edge.Key, Edge.Value };
        while(OutlineEdges.Num() > 0)
        {
            auto V0 = Indices[0];
            auto LastV = Indices[Indices.Num() - 1];
            auto Index = OutlineEdges.IndexOfByPredicate([LastV](const TTuple<int32, int32>& E) { return E.Key == LastV || E.Value == LastV; });
            if (Index < 0)
                break;
            auto E = OutlineEdges[Index];
            OutlineEdges.RemoveAt(Index);
            // 1周した
            if (E.Key == V0 || E.Value == V0)
                break;
            Indices.Add(E.Key == LastV ? E.Value : E.Key);
        }
        Result.Add(Indices);      
    }

    return Result;
}

FSubDividedCityObjectSubMesh FSubDividedCityObjectSubMesh::DeepCopy() const {
    FSubDividedCityObjectSubMesh Result;
    Result.Triangles = Triangles;
    return Result;
}

void FSubDividedCityObjectMesh::VertexReduction() {
    if (Vertices.Num() == 0) {
        return;
    }

    // Create vertex mapping
    TMap<FVector, int32> UniqueVertices;
    TArray<int32> OldToNewIndices;
    OldToNewIndices.SetNum(Vertices.Num());

    TArray<FVector> NewVertices;
    int32 NewIndex = 0;

    for (int32 i = 0; i < Vertices.Num(); ++i) {
        const FVector& Vertex = Vertices[i];
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
    Vertices = NewVertices;

    // Update indices in submeshes
    for (auto& SubMesh : SubMeshes) {
        for (int32 i = 0; i < SubMesh.Triangles.Num(); ++i) {
            SubMesh.Triangles[i] = OldToNewIndices[SubMesh.Triangles[i]];
        }
    }
}

void FSubDividedCityObjectMesh::Separate() {
    TArray<FSubDividedCityObjectSubMesh> NewSubMeshes;
    for (const auto& SubMesh : SubMeshes) {
        NewSubMeshes.Append(SubMesh.Separate());
    }

    SubMeshes = NewSubMeshes;
}

FSubDividedCityObjectMesh FSubDividedCityObjectMesh::DeepCopy() const {
    FSubDividedCityObjectMesh Result;
    Result.Vertices = Vertices;
    for (const auto& SubMesh : SubMeshes) {
        Result.SubMeshes.Add(SubMesh.DeepCopy());
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

FSubDividedCityObject::FSubDividedCityObject(UPLATEAUCityObjectGroup* Co, const plateau::polygonMesh::Model& PlateauModel, TMap<FString, FPLATEAUCityObject>& CityObj)
    : SelfRoadType(ERRoadTypeMask::Empty)
    , ParentRoadType(ERRoadTypeMask::Empty) {

    CityObjectGroup = Co;
    for (int32 i = 0; i < PlateauModel.getRootNodeCount(); ++i) {
        auto& Node = PlateauModel.getRootNodeAt(i);
        FSubDividedCityObject Child(Co, Node, CityObj, ERRoadTypeMask::Empty);
        Children.Add(Child);
    }
}

FSubDividedCityObject::FSubDividedCityObject(UPLATEAUCityObjectGroup* Co,
    const plateau::polygonMesh::Node& PlateauNode,
    TMap<FString, FPLATEAUCityObject>& CityObj,
    ERRoadTypeMask ParentTypeMask)
    : Name(PlateauNode.getName().c_str())
    , SelfRoadType(ERRoadTypeMask::Empty)
    , ParentRoadType(ParentTypeMask) {

    CityObjectGroup = Co;
    // Convert mesh data
    if (PlateauNode.getMesh()) {
        FSubDividedCityObjectMesh Mesh;
        // Convert vertices
        const auto& SrcVertices = PlateauNode.getMesh()->getVertices();
        for (int32 i = 0; i < SrcVertices.size(); ++i) 
        {
            auto&& v = SrcVertices[i];
            Mesh.Vertices.Add(FVector(v.x, v.y, v.z));
        }

        // Convert submeshes
        const auto& SrcSubMeshes = PlateauNode.getMesh()->getSubMeshes();
        const auto& indices = PlateauNode.getMesh()->getIndices();
        for (const auto& SrcSubMesh : SrcSubMeshes) {
            FSubDividedCityObjectSubMesh SubMesh;
            // #NOTE : PLATEAUMeshLoader.cpp参考
            for (auto Index = SrcSubMesh.getStartIndex(); Index <= SrcSubMesh.getEndIndex(); Index++) 
            {                
                SubMesh.Triangles.Add(indices[Index]);
            }
            Mesh.SubMeshes.Add(SubMesh);
        }
        Mesh.VertexReduction();
        Meshes.Add(Mesh);
    }

    const FString DesiredName = FString(UTF8_TO_TCHAR(PlateauNode.getName().c_str()));
    if(auto Tmp = CityObj.Find(DesiredName))
        CityObject = *Tmp;

    SelfRoadType = GetRoadTypeFromCityObject(CityObject);
    // Process child nodes
    for (int32 i = 0; i < PlateauNode.getChildCount(); ++i) {
        auto& ChildNode = PlateauNode.getChildAt(i);
        FSubDividedCityObject Child(Co, ChildNode, CityObj, SelfRoadType);
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

ERRoadTypeMask FSubDividedCityObject::GetRoadTypeFromCityObject(const FPLATEAUCityObject& CityObject)
{
    auto Ret = ERRoadTypeMask::Empty;

    // LOD3からチェックする
    auto& AttrMap = CityObject.Attributes.AttributeMap;
    if(auto TranFunc = AttrMap.Find("tran:function"))
    {
        auto&& Str = TranFunc->StringValue;
        if(Str == TEXT("車道部") || Str == TEXT("車道交差部"))
        {
            Ret |= ERRoadTypeMask::Road;
        }

        if (Str == TEXT("歩道部")) {
            Ret |= ERRoadTypeMask::SideWalk;
        }

        if(Str == TEXT("島"))
        {
            Ret |= ERRoadTypeMask::Median;
        }

        if(Str.Contains(TEXT("高速")))
        {
            Ret |= ERRoadTypeMask::HighWay;
        }
    }


    // LOD1
    if(auto TranClass = AttrMap.Find("tran:class"))
    {
        auto&& Str = TranClass->StringValue;
        if (Str == TEXT("道路")) {
            Ret |= ERRoadTypeMask::Road;
        }
    }


    if (Ret == ERRoadTypeMask::Empty)
        Ret |= ERRoadTypeMask::Undefined;
    return Ret;
}
