#include "RoadNetwork/RGraph/RGraphFactory.h"

#include "Component/PLATEAUCityObjectGroup.h"
#include "RoadNetwork/RnDef.h"
#include "RoadNetwork/CityObject/SubDividedCityObject.h"
#include "RoadNetwork/RGraph/RGraph.h"
#include "RoadNetwork/RGraph/RGraphEx.h"

namespace
{
    struct FEdgeKey {
        RGraphRef_t<URVertex> V0;
        RGraphRef_t<URVertex> V1;

        FEdgeKey(RGraphRef_t<URVertex> InV0, RGraphRef_t<URVertex> InV1)
            : V0(InV0), V1(InV1)
        {
            if (V0 > V1) {
                Swap(V0, V1);
            }
        }

        bool operator==(const FEdgeKey& Other) const {
            return (V0 == Other.V0 && V1 == Other.V1) || (V0 == Other.V1 && V1 == Other.V0);
        }

        friend uint32 GetTypeHash(const FEdgeKey& Key) {
            return HashCombine(GetTypeHash(Key.V0), GetTypeHash(Key.V1));
        }
    };
}

RGraphRef_t<URGraph> FRGraphFactoryEx::CreateGraph(const FRGraphFactory& Factory,
    const TArray<FSubDividedCityObject>& CityObjects)
{
    auto Graph = RGraphNew<URGraph>();

    TMap<FVector, RGraphRef_t<URVertex>> VertexMap;
    TMap<FEdgeKey, RGraphRef_t<UREdge>> EdgeMap;
    auto OrigVertexCount = 0;
    auto OrigEdgeCount = 0;
    for (auto& CityObject : CityObjects) {
        if (CityObject.CityObjectGroup == nullptr) {
            continue;
        }

        auto&& LODLevel = CityObject.CityObjectGroup->MinLOD;
        auto&& RoadType = CityObject.GetRoadType(true);
        // transformを適用する
        auto&& tr = CityObject.CityObjectGroup->GetComponentTransform();
        for (auto&& mesh : CityObject.Meshes) {
            auto&& face = RGraphNew<URFace>(Graph, CityObject.CityObjectGroup.Get(), RoadType, LODLevel);

            TArray<RGraphRef_t<URVertex>> vertices;
            for (auto&& LocalPos : mesh.Vertices) {
                auto WorldPos = tr.TransformPosition(LocalPos);

                if (VertexMap.Contains(WorldPos) == false) {
                    VertexMap.Add(WorldPos, RGraphNew<URVertex>(WorldPos));
                }

                vertices.Add(VertexMap[WorldPos]);
            }
            OrigVertexCount += mesh.Vertices.Num();
            for (auto&& s : mesh.SubMeshes) {
                auto AddEdge = [&EdgeMap, &face, &OrigEdgeCount](RGraphRef_t<URVertex> V0, RGraphRef_t<URVertex> V1) {

                    auto key = FEdgeKey(V0, V1);
                    if (EdgeMap.Contains(key) == false) {
                        EdgeMap.Add(key, RGraphNew<UREdge>(key.V0, key.V1));
                    }
                    face->AddEdge(EdgeMap[key]);
                    OrigEdgeCount++;
                    };

                if (Factory.bUseCityObjectOutline) {
                    auto&& indexTable = s.CreateOutlineIndices();
                    for (auto&& indices : indexTable) {
                        for (auto&& i = 0; i < indices.Num(); i++) {
                            AddEdge(vertices[indices[i]], vertices[indices[(i + 1) % indices.Num()]]);
                        }
                    }
                }
                else {
                    for (auto&& i = 0; i < s.Triangles.Num(); i += 3) {
                        AddEdge(vertices[s.Triangles[i + 0]], vertices[s.Triangles[i + 1]]);
                        AddEdge(vertices[s.Triangles[i + 1]], vertices[s.Triangles[i + 2]]);
                        AddEdge(vertices[s.Triangles[i + 2]], vertices[s.Triangles[i]]);
                    }
                }

            }
            Graph->AddFace(face);
        }
    }

    if (Factory.bReductionOnCreate) {
        FRGraphEx::Optimize(Graph, Factory.MergeCellSize, Factory.MergeCellLength,
            Factory.RemoveMidPointTolerance, Factory.Lod1HeightTolerance);
    }

    return Graph;
}
