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
#include "CoreMinimal.h"
#include "RnLineString.h"
#include "RnPoint.h"
#include "../RnDef.h"

class RnWay
{
public:
    class PointIterator
    {
    public:
        PointIterator(const RnWay* InWay, int32 index)
            : Way(InWay)
            , Index(index) {
        }

        RnRef_t<RnPoint> operator*() const {
            return Way->GetPoint(Index);
        }

        PointIterator& operator++() {
            ++Index;
            return *this;
        }

        bool operator!=(const PointIterator& Other) const {
            return Index != Other.Index;
        }
        const RnWay* Way;
        int32 Index;
    };

    class PointsEnumerator
    {
    public:
        PointsEnumerator(const RnWay* InWay)
            : Way(InWay)
        {
        }
        PointIterator begin() const {
            return PointIterator(Way, 0);
        }

        PointIterator end() const {
            return PointIterator(Way, Way->Count());
        }

        const RnWay* Way;
    };

    class VertexIterator {
    public:
        VertexIterator(const RnWay* InWay, int32 index)
            : Way(InWay)
            , Index(index) {
        }

        FVector operator*() const {
            return Way->GetVertex(Index);
        }

        VertexIterator& operator++() {
            ++Index;
            return *this;
        }

        bool operator!=(const VertexIterator& Other) const {
            return Index != Other.Index;
        }
        const RnWay* Way;
        int32 Index;
    };

    class VertexEnumerator {
    public:
        VertexEnumerator(const RnWay* InWay)
            : Way(InWay) {
        }
        VertexIterator begin() const {
            return VertexIterator(Way, 0);
        }

        VertexIterator end() const {
            return VertexIterator(Way, Way->Count());
        }

        const RnWay* Way;
    };

public:
    RnWay();
    RnWay(const RnRef_t<RnLineString>& InLineString, bool bInIsReversed = false, bool bInIsReverseNormal = false);

    bool IsReversed;
    bool IsReverseNormal;
    RnRef_t<RnLineString> LineString;


    PointsEnumerator GetPoints() const
    {
        return PointsEnumerator(this);    
    }

    VertexEnumerator GetVertices() const {
        return VertexEnumerator(this);
    }

    int32 Count() const;
    bool IsValid() const;

    RnRef_t<RnPoint> GetPoint(int32 Index) const;
    RnRef_t<RnPoint> SetPoint(int32 Index, const RnRef_t<RnPoint>& Point);

    FVector GetVertex(int32 Index) const;
    FVector operator[](int32 Index) const;

    RnRef_t<RnWay> ReversedWay() const;
    void Reverse(bool KeepNormalDir);

    FVector GetVertexNormal(int32 VertexIndex) const;
    FVector GetEdgeNormal(int32 StartVertexIndex) const;

    void MoveAlongNormal(float Offset);
    void Move(const FVector& Offset);

    RnRef_t<RnWay> Clone(bool CloneVertex) const;

    TArray<FLineSegment2D> GetEdges2D() const;

    int32 FindPoint(const RnRef_t<RnPoint>& Point) const;
    int32 FindPointIndex(const RnRef_t<RnPoint>& Point) const;
    void GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;
    bool IsValidOrDefault() const;
    float CalcLength() const;
    float CalcLength(float StartIndex, float EndIndex) const;
    void AppendBack2LineString(const RnRef_t<RnWay>& Back);
    void AppendFront2LineString(const RnRef_t<RnWay>& Front);
    bool IsOutSide(const FVector& V, FVector& OutNearest, float& OutDistance) const;
    void MoveLerpAlongNormal(const FVector& StartOffset, const FVector& EndOffset);
    FVector GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPoint(float Offset, bool Reverse, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPoint(float Offset, bool Reverse) const;
    float GetDistance2D(const RnRef_t<RnWay>& Other, EAxisPlane Plane = FRnDef::Plane) const;

    // 線分の距離をp : (1-p)で分割した点を返す
    FVector GetLerpPoint(float P) const;
    float GetLerpPoint(float P, FVector& OutMidPoint) const;

    // 同じLineStringを参照しているかどうか
    bool IsSameLineReference(const RnRef_t<RnWay>& Other) const;

    // 同じ頂点列を持っているかどうか
    bool IsSameLineSequence(const RnRef_t<RnWay>& Other) const;

private:
    int32 ToRawIndex(int32 Index, bool AllowMinus = false) const;
    int32 SwitchIndex(int32 Index) const;
    float SwitchIndex(float Index) const;
};
