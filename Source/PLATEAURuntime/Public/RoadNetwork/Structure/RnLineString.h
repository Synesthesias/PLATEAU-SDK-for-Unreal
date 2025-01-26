#pragma once

#include "CoreMinimal.h"
#include "RnPoint.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "RoadNetwork/GeoGraph/AxisPlane.h"
#include "RoadNetwork/GeoGraph/LineSegment2D.h"
#include "RoadNetwork/GeoGraph/LineSegment3D.h"
#include <memory>

#include "RnLineString.generated.h"
UCLASS()
class URnLineString : public UObject
{
    GENERATED_BODY()
public:
    URnLineString();
    URnLineString(int32 InitialSize);
    URnLineString(const TArray<TRnRef_T<URnPoint>>& InPoints);
    void Init();
    void Init(int32 InitialSize);
    void Init(const TArray<TRnRef_T<URnPoint>>& InPoints);

    const TArray<TRnRef_T<URnPoint>>& GetPoints() const
    {
        return Points;    
    }

    TArray<TRnRef_T<URnPoint>>& GetPoints() {
        return Points;
    }

    void SetPoints(const TArray<TRnRef_T<URnPoint>>& InPoints) {
        Points = InPoints;
    }

    int32 Count() const;
    bool IsValid() const;

    TArray<TRnRef_T<URnLineString>> Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector = nullptr);

    TArray<TRnRef_T<URnLineString>> SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint = false) const;
    void AddFrontPoint(TRnRef_T<URnPoint> Point);

    bool Contains(TRnRef_T<URnPoint> Point) const;
    void GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;
    float GetDistance2D(const TRnRef_T<URnLineString> Other, EAxisPlane Plane = EAxisPlane::Xy) const;

    void AddPointOrSkip(TRnRef_T<URnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);
    void AddPointFrontOrSkip(TRnRef_T<URnPoint> Point, float DistanceEpsilon = 0.0f, float DegEpsilon = 0.5f, float MidPointTolerance = 0.3f);

    FVector GetVertexNormal(int32 VertexIndex) const;
    FVector GetEdgeNormal(int32 StartVertexIndex) const;

    TRnRef_T<URnLineString> Clone(bool CloneVertex) const;
    int32 ReplacePoint(TRnRef_T<URnPoint> OldPoint, TRnRef_T<URnPoint> NewPoint);

    float CalcLength() const;
    float CalcLength(float StartIndex, float EndIndex) const;
    float CalcTotalAngle2D() const;



    TArray<FLineSegment2D> GetEdges2D(EAxisPlane axis = FPLATEAURnDef::Plane) const;
    TArray<FLineSegment3D> GetEdges() const;

    static TRnRef_T<URnLineString> Create(const TArray<TRnRef_T<URnPoint>>& Vertices, bool RemoveDuplicate = true);
    static TRnRef_T<URnLineString> Create(const TArray<FVector>& Vertices, bool RemoveDuplicate = true);
    static bool Equals(const TRnRef_T<URnLineString> X, const TRnRef_T<URnLineString> Y);

    FVector operator[](int32 Index) const;

    FVector GetVertex(int32 Index) const;

    TRnRef_T<URnPoint> GetPoint(int32 Index) const;

    void SetPoint(int32 Index, const TRnRef_T<URnPoint>& Point);

    FVector GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;

    TArray<TTuple<float, FVector>> GetIntersectionBy2D(
        const FLineSegment3D& LineSegment,
        EAxisPlane Plane) const;

private:
    UPROPERTY()
    TArray<TObjectPtr<URnPoint>> Points;
};
