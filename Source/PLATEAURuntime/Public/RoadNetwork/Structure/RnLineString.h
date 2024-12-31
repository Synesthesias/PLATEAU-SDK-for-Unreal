#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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
    RnLineString(const std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>& InPoints);

    std::shared_ptr<TArray<std::shared_ptr<RnPoint>>> Points;

    int32 Count() const;
    bool IsValid() const;

    TArray<std::shared_ptr<RnLineString>> Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector = nullptr);

    TArray<std::shared_ptr<RnLineString>> SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint = false) const;
    void AddFrontPoint(std::shared_ptr<RnPoint> Point);

    bool Contains(std::shared_ptr<RnPoint> Point) const;
    void GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;
    float GetDistance2D(const std::shared_ptr<RnLineString> Other, EAxisPlane Plane = EAxisPlane::Xy) const;

    void AddPointOrSkip(std::shared_ptr<RnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);
    void AddPointFrontOrSkip(std::shared_ptr<RnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);

    FVector GetVertexNormal(int32 VertexIndex) const;
    FVector GetEdgeNormal(int32 StartVertexIndex) const;

    std::shared_ptr<RnLineString> Clone(bool CloneVertex) const;
    int32 ReplacePoint(std::shared_ptr<RnPoint> OldPoint, std::shared_ptr<RnPoint> NewPoint);

    float CalcLength() const;
    float CalcLength(float StartIndex, float EndIndex) const;
    float CalcTotalAngle2D() const;



    TArray<FLineSegment2D> GetEdges2D(EAxisPlane axis = FRnDef::Plane) const;
    TArray<FLineSegment3D> GetEdges() const;

    static std::shared_ptr<RnLineString> Create(const std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>& Vertices, bool RemoveDuplicate = true);
    static std::shared_ptr<RnLineString> Create(const TArray<FVector>& Vertices, bool RemoveDuplicate = true);
    static bool Equals(const std::shared_ptr<RnLineString> X, const std::shared_ptr<RnLineString> Y);

    FVector operator[](int32 Index) const;

    FVector Get(int32 Index) const;

    std::shared_ptr<RnPoint> GetPoint(int32 Index) const;
};
