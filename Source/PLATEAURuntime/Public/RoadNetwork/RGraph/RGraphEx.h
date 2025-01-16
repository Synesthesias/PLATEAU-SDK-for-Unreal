#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "RGraph.h"
#include <memory>
#include <functional>

class FSubDividedCityObject;
class UPLATEAUCityObjectGroup;
class FRGraphEx {
public:
    static void RemoveInnerVertex(RGraphRef_t<URFace> Face);
    static void RemoveInnerVertex(RGraphRef_t<URGraph> Graph);
    static TSet<RGraphRef_t<URVertex>> AdjustSmallLodHeight(RGraphRef_t<URGraph> Graph, float MergeCellSize, int32 MergeCellLength, float HeightTolerance);
    static void VertexReduction(RGraphRef_t<URGraph> Graph, float MergeCellSize, int32 MergeCellLength, float MidPointTolerance);
    static void EdgeReduction(RGraphRef_t<URGraph> Graph);
    static void MergeIsolatedVertices(RGraphRef_t<URGraph> Graph);
    static void MergeIsolatedVertex(RGraphRef_t<URFace> Face);
    static TArray<RGraphRef_t<URFaceGroup>> GroupBy(RGraphRef_t<URGraph> Graph, TFunction<bool(RGraphRef_t<URFace>, RGraphRef_t<URFace>)> IsMatch);
    static void Optimize(RGraphRef_t<URGraph> Graph, float MergeCellSize, int32 MergeCellLength, float MidPointTolerance, float Lod1HeightTolerance);
    static void InsertVertexInNearEdge(RGraphRef_t<URGraph> Graph, float Tolerance);
    static void InsertVerticesInEdgeIntersection(RGraphRef_t<URGraph> Graph, float HeightTolerance);
    static TArray<RGraphRef_t<UREdge>> InsertVertices(RGraphRef_t<UREdge> Edge, const TArray<RGraphRef_t<URVertex>>& Vertices);
    static void SeparateFaces(RGraphRef_t<URGraph> Graph);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVertices(RGraphRef_t<URFace> Face);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVertices(RGraphRef_t<URFaceGroup> FaceGroup, TFunction<bool(RGraphRef_t<URFace>)> Predicate);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVerticesByCityObjectGroup(RGraphRef_t<URGraph> Graph, UPLATEAUCityObjectGroup* CityObjectGroup, ERRoadTypeMask RoadTypes, ERRoadTypeMask RemoveRoadTypes);
    static TArray<RGraphRef_t<URVertex>> ComputeConvexHullVertices(RGraphRef_t<URFace> Face);
    static bool IsShareEdge(RGraphRef_t<URFace> A, RGraphRef_t<URFace> B);
    static TSet<RGraphRef_t<URVertex>> CreateVertexSet(RGraphRef_t<URFace> Face);
};
