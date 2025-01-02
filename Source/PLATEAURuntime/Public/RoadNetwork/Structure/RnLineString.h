#pragma once

#include "CoreMinimal.h"
#include "RnPoint.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "RoadNetwork/RnDef.h"
#include "RoadNetwork/GeoGraph/AxisPlane.h"
#include "RoadNetwork/GeoGraph/LineSegment2D.h"
#include "RoadNetwork/GeoGraph/LineSegment3D.h"
#include <memory>

class RnLineString {
public:
    RnLineString();
    RnLineString(int32 InitialSize);
    RnLineString(const TSharedPtr<TArray<RnRef_t<RnPoint>>>& InPoints);

    TSharedPtr<TArray<RnRef_t<RnPoint>>> Points;

    int32 Count() const;
    bool IsValid() const;

    TArray<RnRef_t<RnLineString>> Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector = nullptr);

    TArray<RnRef_t<RnLineString>> SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint = false) const;
    void AddFrontPoint(RnRef_t<RnPoint> Point);

    bool Contains(RnRef_t<RnPoint> Point) const;
    void GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;
    float GetDistance2D(const RnRef_t<RnLineString> Other, EAxisPlane Plane = EAxisPlane::Xy) const;

    void AddPointOrSkip(RnRef_t<RnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);
    void AddPointFrontOrSkip(RnRef_t<RnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);

    FVector GetVertexNormal(int32 VertexIndex) const;
    FVector GetEdgeNormal(int32 StartVertexIndex) const;

    RnRef_t<RnLineString> Clone(bool CloneVertex) const;
    int32 ReplacePoint(RnRef_t<RnPoint> OldPoint, RnRef_t<RnPoint> NewPoint);

    float CalcLength() const;
    float CalcLength(float StartIndex, float EndIndex) const;
    float CalcTotalAngle2D() const;



    TArray<FLineSegment2D> GetEdges2D(EAxisPlane axis = FRnDef::Plane) const;
    TArray<FLineSegment3D> GetEdges() const;

    static RnRef_t<RnLineString> Create(const TSharedPtr<TArray<RnRef_t<RnPoint>>>& Vertices, bool RemoveDuplicate = true);
    static RnRef_t<RnLineString> Create(const TArray<FVector>& Vertices, bool RemoveDuplicate = true);
    static bool Equals(const RnRef_t<RnLineString> X, const RnRef_t<RnLineString> Y);

    FVector operator[](int32 Index) const;

    FVector GetVertex(int32 Index) const;

    RnRef_t<RnPoint> GetPoint(int32 Index) const;

    void SetPoint(int32 Index, const RnRef_t<RnPoint>& Point);

    FVector GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;

    TArray<TTuple<float, FVector>> GetIntersectionBy2D(
        const FLineSegment3D& LineSegment,
        EAxisPlane Plane) const;
};
