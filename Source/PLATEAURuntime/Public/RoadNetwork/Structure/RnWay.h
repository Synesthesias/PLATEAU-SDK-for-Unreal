#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RnPoint.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "RoadNetwork/GeoGraph/AxisPlane.h"
#include "RoadNetwork/GeoGraph/LineSegment2D.h"
#include "RoadNetwork/GeoGraph/LineSegment3D.h"
#include "CoreMinimal.h"
#include "RnLineString.h"
#include "RnPoint.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include "RnWay.generated.h"
UCLASS()
class URnWay : public UObject
{
    GENERATED_BODY()
public:
    class PointIterator
    {
    public:
        PointIterator(const URnWay* InWay, int32 index)
            : Way(InWay)
            , Index(index) {
        }

        TRnRef_T<URnPoint> operator*() const {
            return Way->GetPoint(Index);
        }

        PointIterator& operator++() {
            ++Index;
            return *this;
        }

        bool operator!=(const PointIterator& Other) const {
            return Index != Other.Index;
        }
        const URnWay* Way;
        int32 Index;
    };

    class PointsEnumerator
    {
    public:
        PointsEnumerator(const URnWay* InWay)
            : Way(InWay)
        {
        }
        PointIterator begin() const {
            return PointIterator(Way, 0);
        }

        PointIterator end() const {
            return PointIterator(Way, Way->Count());
        }

        const URnWay* Way;
    };

    class VertexIterator {
    public:
        VertexIterator(const URnWay* InWay, int32 index)
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
        const URnWay* Way;
        int32 Index;
    };

    class VertexEnumerator {
    public:
        VertexEnumerator(const URnWay* InWay)
            : Way(InWay) {
        }
        VertexIterator begin() const {
            return VertexIterator(Way, 0);
        }

        VertexIterator end() const {
            return VertexIterator(Way, Way->Count());
        }

        TArray<FVector> ToArray() const {
            TArray<FVector> Result;
            for (int32 i = 0; i < Way->Count(); ++i) {
                Result.Add(Way->GetVertex(i));
            }
            return Result;
        }

        const URnWay* Way;
    };

public:
    URnWay();
    URnWay(const TRnRef_T<URnLineString>& InLineString, bool bInIsReversed = false, bool bInIsReverseNormal = false);
    void Init();
    void Init(const TRnRef_T<URnLineString>& InLineString, bool bInIsReversed = false, bool bInIsReverseNormal = false);


    // URnPointのイテレータ
    PointsEnumerator GetPoints() const
    {
        return PointsEnumerator(this);    
    }

    // FVectorのイテレータ
    VertexEnumerator GetVertices() const {
        return VertexEnumerator(this);
    }

    int32 Count() const;
    bool IsValid() const;

    TRnRef_T<URnPoint> GetPoint(int32 Index) const;
    TRnRef_T<URnPoint> SetPoint(int32 Index, const TRnRef_T<URnPoint>& Point);

    FVector GetVertex(int32 Index) const;
    FVector operator[](int32 Index) const;

    TRnRef_T<URnWay> ReversedWay() const;
    void Reverse(bool KeepNormalDir);

    FVector GetVertexNormal(int32 VertexIndex) const;
    FVector GetEdgeNormal(int32 StartVertexIndex) const;

    void MoveAlongNormal(float Offset);
    void Move(const FVector& Offset);

    TRnRef_T<URnWay> Clone(bool CloneVertex) const;

    TArray<FLineSegment2D> GetEdges2D() const;

    int32 FindPoint(const TRnRef_T<URnPoint>& Point) const;
    int32 FindPointIndex(const TRnRef_T<URnPoint>& Point) const;
    void GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const;
    bool IsValidOrDefault() const;
    float CalcLength() const;
    float CalcLength(float StartIndex, float EndIndex) const;
    void AppendBack2LineString(const TRnRef_T<URnWay>& Back);
    void AppendFront2LineString(const TRnRef_T<URnWay>& Front);
    bool IsOutSide(const FVector& V, FVector& OutNearest, float& OutDistance) const;
    void MoveLerpAlongNormal(const FVector& StartOffset, const FVector& EndOffset);
    FVector GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPoint(float Offset, bool Reverse, int32& OutStartIndex, int32& OutEndIndex) const;
    FVector GetAdvancedPoint(float Offset, bool Reverse) const;
    float GetDistance2D(const TRnRef_T<URnWay>& Other, EAxisPlane Plane = FPLATEAURnDef::Plane) const;

    // 線分の距離をp : (1-p)で分割した点を返す
    FVector GetLerpPoint(float P) const;
    float GetLerpPoint(float P, FVector& OutMidPoint) const;

    // 同じLineStringを参照しているかどうか
    bool IsSameLineReference(const TRnRef_T<URnWay>& Other) const;

    // 同じ頂点列を持っているかどうか
    bool IsSameLineSequence(const TRnRef_T<URnWay>& Other) const;

    // 自身をnum分割して返す. 分割できない(頂点空）の時は空リストを返す.
    // insertNewPoint=trueの時はselfにも新しい点を追加する
    TArray<TRnRef_T<URnWay>> Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector = nullptr);
private:
    int32 ToRawIndex(int32 Index, bool AllowMinus = false) const;
    int32 SwitchIndex(int32 Index) const;
    float SwitchIndex(float Index) const;

public:
    UPROPERTY()
    bool IsReversed;

    UPROPERTY()
    bool IsReverseNormal;

    UPROPERTY()
    URnLineString* LineString;
};

struct FRnWayEx
{
    // Selfの長さを計算する
    static float CalcLengthOrDefault(const URnWay* Self);

    // Self != nullptr && Self->IsValid()
    static bool IsValidWayOrDefault(const URnWay* Self);
};