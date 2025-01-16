#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include "RGraph.h"
#include <memory>
#include <functional>

class FSubDividedCityObject;
class UPLATEAUCityObjectGroup;
class FRGraphHelper {
public:
    static void RemoveInnerVertex(TSharedPtr<FRFace> Face);
    static void RemoveInnerVertex(TSharedPtr<FRGraph> Graph);
    static TSet<TSharedPtr<FRVertex>> AdjustSmallLodHeight(TSharedPtr<FRGraph> Graph, float MergeCellSize, int32 MergeCellLength, float HeightTolerance);
    static void VertexReduction(TSharedPtr<FRGraph> Graph, float MergeCellSize, int32 MergeCellLength, float MidPointTolerance);
    static void EdgeReduction(TSharedPtr<FRGraph> Graph);
    static void MergeIsolatedVertices(TSharedPtr<FRGraph> Graph);
    static void MergeIsolatedVertex(TSharedPtr<FRFace> Face);
    static TArray<TSharedPtr<FRFaceGroup>> GroupBy(TSharedPtr<FRGraph> Graph, TFunction<bool(TSharedPtr<FRFace>, TSharedPtr<FRFace>)> IsMatch);
    static void Optimize(TSharedPtr<FRGraph> Graph, float MergeCellSize, int32 MergeCellLength, float MidPointTolerance, float Lod1HeightTolerance);
    static void InsertVertexInNearEdge(TSharedPtr<FRGraph> Graph, float Tolerance);
    static void InsertVerticesInEdgeIntersection(TSharedPtr<FRGraph> Graph, float HeightTolerance);
    static TArray<TSharedPtr<FREdge>> InsertVertices(TSharedPtr<FREdge> Edge, const TArray<TSharedPtr<FRVertex>>& Vertices);
    static void SeparateFaces(TSharedPtr<FRGraph> Graph);
    static TArray<TSharedPtr<FRVertex>> ComputeOutlineVertices(TSharedPtr<FRFace> Face);
    static TArray<TSharedPtr<FRVertex>> ComputeOutlineVertices(TSharedPtr<FRFaceGroup> FaceGroup, TFunction<bool(TSharedPtr<FRFace>)> Predicate);
    static TArray<TSharedPtr<FRVertex>> ComputeOutlineVerticesByCityObjectGroup(TSharedPtr<FRGraph> Graph, UPLATEAUCityObjectGroup* CityObjectGroup, ERRoadTypeMask RoadTypes, ERRoadTypeMask RemoveRoadTypes);
    static TArray<TSharedPtr<FRVertex>> ComputeConvexHullVertices(TSharedPtr<FRFace> Face);
    static bool IsShareEdge(TSharedPtr<FRFace> A, TSharedPtr<FRFace> B);
    static TSet<TSharedPtr<FRVertex>> CreateVertexSet(TSharedPtr<FRFace> Face);

    static TSharedPtr<FRGraph> CreateGraph(const TArray<TSharedPtr<FSubDividedCityObject>>& CityObjects, bool useOutline);

};
