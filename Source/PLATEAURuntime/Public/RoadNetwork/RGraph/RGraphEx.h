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
    static TSet<RGraphRef_t<URVertex>> AdjustSmallLodHeight(RGraphRef_t<URGraph> Graph, float MergeCellSizeMeter, int32 MergeCellLength, float HeightToleranceMeter);
    static void VertexReduction(RGraphRef_t<URGraph> Graph, float MergeCellSizeMeter, int32 MergeCellLength, float MidPointToleranceMeter);
    static void EdgeReduction(RGraphRef_t<URGraph> Graph);
    static void MergeIsolatedVertices(RGraphRef_t<URGraph> Graph);
    static void MergeIsolatedVertex(RGraphRef_t<URFace> Face);
    static TArray<RGraphRef_t<URFaceGroup>> GroupBy(RGraphRef_t<URGraph> Graph, TFunction<bool(RGraphRef_t<URFace>, RGraphRef_t<URFace>)> IsMatch);
    static void InsertVertexInNearEdge(RGraphRef_t<URGraph> Graph, float ToleranceMeter);
    static void InsertVerticesInEdgeIntersection(RGraphRef_t<URGraph> Graph, float HeightToleranceMeter);
    static TArray<RGraphRef_t<UREdge>> InsertVertices(RGraphRef_t<UREdge> Edge, TArray<RGraphRef_t<URVertex>> Vertices);
    static void SeparateFaces(RGraphRef_t<URGraph> Graph);
    static void SeparateFace(RGraphRef_t<URFace> Face);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVertices(const TArray<RGraphRef_t<URFace>>& Faces);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVertices(RGraphRef_t<URFace> Face);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVertices(RGraphRef_t<URFaceGroup> FaceGroup, TFunction<bool(RGraphRef_t<URFace>)> Predicate);
    static TArray<RGraphRef_t<URVertex>> ComputeOutlineVerticesByCityObjectGroup(RGraphRef_t<URGraph> Graph, UPLATEAUCityObjectGroup* CityObjectGroup, ERRoadTypeMask RoadTypes, ERRoadTypeMask RemoveRoadTypes);
    static TArray<RGraphRef_t<URVertex>> ComputeConvexHullVertices(RGraphRef_t<URFace> Face);
    static bool IsShareEdge(RGraphRef_t<URFace> A, RGraphRef_t<URFace> B);
    static TSet<RGraphRef_t<URVertex>> CreateVertexSet(RGraphRef_t<URFace> Face);
    static void RemoveIsolatedEdgeFromFace(RGraphRef_t<URGraph> Self);
    static TSet<RGraphRef_t<UREdge>> RemoveIsolatedEdge(RGraphRef_t<URFace> Self);
};
