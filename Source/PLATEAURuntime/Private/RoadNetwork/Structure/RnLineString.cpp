#include "RoadNetwork/Structure/RnLineString.h"

#include <optional>

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Util/Vector2DEx.h"

URnLineString::URnLineString() {
}

URnLineString::URnLineString(int32 InitialSize)
{
    Points.SetNum(InitialSize);
}

URnLineString::URnLineString(const TArray<TRnRef_T<URnPoint>>& InPoints)
{
    Points = InPoints;
}

void URnLineString::Init()
{
    
}

void URnLineString::Init(int32 InitialSize)
{
    Points.SetNum(InitialSize);
}

void URnLineString::Init(const TArray<TRnRef_T<URnPoint>>& InPoints)
{
    Points = InPoints;
}


int32 URnLineString::Count() const
{ return Points.Num(); }

bool URnLineString::IsValid() const
{ return Points.Num() >= 2; }

void URnLineString::AddPointOrSkip(TRnRef_T<URnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
    if (!Point) 
        return;

    if (Points.Num() > 0) {
        const float SqrDistanceThreshold = DistanceEpsilon < 0.0f ? -1.0f : DistanceEpsilon * DistanceEpsilon;
        if (URnPoint::Equals(Points.Last(), Point, SqrDistanceThreshold))
            return;
    }

    Points.Add(Point);
}

FVector URnLineString::GetEdgeNormal(int32 StartVertexIndex) const {
    const FVector P0 = (*this)[StartVertexIndex];
    const FVector P1 = (*this)[StartVertexIndex + 1];
    return FVector::CrossProduct(FVector::UpVector, P1 - P0).GetSafeNormal() * -1.0f;
}

TRnRef_T<URnLineString> URnLineString::Clone(bool CloneVertex) const {
    auto NewLineString = RnNew<URnLineString>();

    if (CloneVertex) {
        for (const TRnRef_T<URnPoint> Point : Points) {
            NewLineString->Points.Add(Point->Clone());
        }
    }
    else {
        NewLineString->Points = Points;
    }

    return NewLineString;
}

float URnLineString::CalcLength() const {
    if (!IsValid()) return 0.0f;

    float Length = 0.0f;
    for (int32 i = 0; i < Points.Num() - 1; ++i) {
        Length += ((Points)[i + 1]->Vertex - (Points)[i]->Vertex).Size();
    }
    return Length;
}

TArray<TRnRef_T<URnLineString>> URnLineString::Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector) {
    TArray<TRnRef_T<URnLineString>> Result;
    if (!IsValid() || Num <= 0) return Result;

    TArray<float> Rates;
    float Rate = 0.0f;
    for (int32 i = 0; i < Num - 1; i++) {
        Rate += RateSelector ? RateSelector(i) : 1.0f / Num;
        Rates.Add(Rate);
    }

    TArray<int32> SplitIndices;
    const float TotalLength = CalcLength();
    float CurrentLength = 0.0f;
    int32 CurrentIndex = 0;

    for (int32 i = 0; i < Points.Num() - 1; i++) {
        const float SegmentLength = (GetVertex(i + 1) - GetVertex(i)).Size();
        const float NextLength = CurrentLength + SegmentLength;

        while (CurrentIndex < Rates.Num() &&
            CurrentLength <= Rates[CurrentIndex] * TotalLength &&
            Rates[CurrentIndex] * TotalLength < NextLength) {
            float T = (Rates[CurrentIndex] * TotalLength - CurrentLength) / SegmentLength;
            if (InsertNewPoint) {
                auto p = RnNew<URnPoint>(FMath::Lerp(GetVertex(i), GetVertex(i+1), T));
                Points.Insert(p, i + 1);
                i++;
            }
            SplitIndices.Add(i);
            CurrentIndex++;
        }
        CurrentLength = NextLength;
    }

    SplitIndices.Add(Points.Num() - 1);
    int32 StartIndex = 0;
    for (const int32 EndIndex : SplitIndices) {
        auto SplitPoints = TArray<TRnRef_T<URnPoint>>();
        for (int32 i = StartIndex; i <= EndIndex; i++) {
            SplitPoints.Add(GetPoint(i));
        }
        Result.Add(RnNew<URnLineString>(SplitPoints));
        StartIndex = EndIndex;
    }

    return Result;
}

TArray<TRnRef_T<URnLineString>> URnLineString::SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint) const {
    TArray<TRnRef_T<URnLineString>> Result;
    if (!IsValid() || Indices.Num() == 0) return Result;

    TArray<int32> SortedIndices = Indices;
    SortedIndices.Sort();

    int32 StartIndex = 0;
    for (int32 EndIndex : SortedIndices) {
        auto SplitPoints = TArray< TRnRef_T<URnPoint>>();
        for (int32 i = StartIndex; i <= EndIndex; ++i) {
            SplitPoints.Add(GetPoint(i));
        }

        if (SplitPoints.Num() >= 2) {
            TRnRef_T<URnLineString> Split = RnNew<URnLineString>(SplitPoints);
            Result.Add(Split);
        }
        StartIndex = EndIndex;
    }

    return Result;
}

void URnLineString::AddFrontPoint(TRnRef_T<URnPoint> Point) {
    if (Point) {
        Points.Insert(Point, 0);
    }
}
void URnLineString::AddPointFrontOrSkip(TRnRef_T<URnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
    if (!Point) return;

    if (Points.Num() > 0) {
        const float SqrDistanceThreshold = DistanceEpsilon < 0.0f ? -1.0f : DistanceEpsilon * DistanceEpsilon;
        if (URnPoint::Equals(GetPoint(0), Point, SqrDistanceThreshold)) {
            return;
        }
    }

    Points.Insert(Point, 0);
}

float URnLineString::CalcLength(float StartIndex, float EndIndex) const {
    if (!IsValid()) return 0.0f;

    const int32 StartIdx = FMath::FloorToInt(StartIndex);
    const int32 EndIdx = FMath::CeilToInt(EndIndex);

    float Length = 0.0f;
    for (int32 i = StartIdx; i < EndIdx && i < Points.Num() - 1; ++i) {
        Length += (GetVertex(i + 1) - GetVertex(i)).Size();
    }

    return Length;
}

float URnLineString::CalcTotalAngle2D() const {
    if (Points.Num() < 3) return 0.0f;

    float TotalAngle = 0.0f;
    std::optional<FVector2D> Last = std::nullopt;
    const auto Edges = FGeoGraphEx::GetEdges(Points, false);
    for (auto It = Edges.begin(); It != Edges.end(); ++It)
    {
        const auto e = *It;
        auto Dir = FLineSegment3D(e.P0->Vertex, e.P1->Vertex).To2D(FRnDef::Plane).GetDirection();
        if(Last.has_value())
        {
            TotalAngle += FVector2DEx::Angle((*Last), Dir);
        }
        Last = Dir;
    }
    return TotalAngle;
}

TArray<FLineSegment2D> URnLineString::GetEdges2D(EAxisPlane axis) const
{
    TArray<FLineSegment2D> Ret;
    const auto Edges = FGeoGraphEx::GetEdges(Points, false);
    for (auto It = Edges.begin(); It != Edges.end(); ++It) 
    {
        const auto e = *It;
        Ret.Add(FLineSegment3D(e.P0->Vertex, e.P0->Vertex).To2D(axis));
    }
    return Ret;
}


TArray<FLineSegment3D> URnLineString::GetEdges() const
{
    TArray<FLineSegment3D> Ret;
    const auto Edges = FGeoGraphEx::GetEdges(Points, false);
    for (auto It = Edges.begin(); It != Edges.end(); ++It) {
        const auto e = *It;
        Ret.Add(FLineSegment3D(e.P0->Vertex, e.P0->Vertex));
    }
    return Ret;
}

bool URnLineString::Contains(TRnRef_T<URnPoint> Point) const {
    return Points.Contains(Point);
}

void URnLineString::GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    OutDistance = MAX_FLT;
    OutPointIndex = 0;

    for (int32 i = 0; i < Points.Num() - 1; ++i) {
        const FVector Start = GetVertex(i);
        const FVector End = GetVertex(i+1);
        const FVector ProjectedPoint = FMath::ClosestPointOnSegment(Pos, Start, End);

        const float Distance = (Pos - ProjectedPoint).Size();
        if (Distance < OutDistance) {
            OutDistance = Distance;
            OutNearest = ProjectedPoint;
            OutPointIndex = i + (ProjectedPoint - Start).Size() / (End - Start).Size();
        }
    }
}

float URnLineString::GetDistance2D(const TRnRef_T<URnLineString> Other, EAxisPlane Plane) const {
    if (!Other || !IsValid() || !Other->IsValid()) return MAX_FLT;

    float MinDistance = MAX_FLT;
    auto SelfEdges = GetEdges2D(Plane);
    auto OtherEdges = Other->GetEdges2D(Plane);
    for (auto& s1 : SelfEdges) 
    {
        for (auto& s2 : OtherEdges) {
            MinDistance = FMath::Min(MinDistance, s1.GetDistance(s2));
        }
    }

    return MinDistance;
}

FVector URnLineString::GetVertexNormal(int32 VertexIndex) const {
    if (Points.Num() <= 1) return FVector::ZeroVector;

    FVector Normal = FVector::ZeroVector;
    int32 Count = 0;

    if (VertexIndex > 0) {
        Normal += GetEdgeNormal(VertexIndex - 1);
        Count++;
    }

    if (VertexIndex < Points.Num() - 1) {
        Normal += GetEdgeNormal(VertexIndex);
        Count++;
    }

    return (Count > 0) ? Normal / Count : FVector::ZeroVector;
}

int32 URnLineString::ReplacePoint(TRnRef_T<URnPoint> OldPoint, TRnRef_T<URnPoint> NewPoint) {
    int32 ReplaceCount = 0;
    for (int32 i = 0; i < Points.Num(); i++) {
        if (GetPoint(i) == OldPoint) {
            (Points)[i] = NewPoint;
            ReplaceCount++;
        }
    }
    return ReplaceCount;
}


TRnRef_T<URnLineString> URnLineString::Create(const TArray<TRnRef_T<URnPoint>>& Vertices,
    bool RemoveDuplicate) {
    auto LineString = RnNew<URnLineString>();
    if (!RemoveDuplicate) 
    {
        for (TRnRef_T<URnPoint> Point : Vertices) {
            LineString->Points.Add(Point);
        }
        return LineString;
    }

    for (TRnRef_T<URnPoint> Point : Vertices) {
        LineString->AddPointOrSkip(Point);
    }

    return LineString;
}

TRnRef_T<URnLineString> URnLineString::Create(const TArray<FVector>& Vertices, bool RemoveDuplicate) {
    auto Points = TArray<TRnRef_T<URnPoint>>();
    for (const FVector& Vertex : Vertices) 
    {
        auto P = RnNew<URnPoint>(Vertex);        
        Points.Add(P);
    }

    return Create(Points, RemoveDuplicate);
}

bool URnLineString::Equals(const TRnRef_T<URnLineString> X, const TRnRef_T<URnLineString> Y) {
    if (X == Y) return true;
    if (!X || !Y) return false;
    if (X->Points.Num() != Y->Points.Num()) return false;

    for (int32 i = 0; i < X->Points.Num(); i++) {
        if (!URnPoint::Equals(X->GetPoint(i), Y->GetPoint(i))) {
            return false;
        }
    }

    return true;
}

FVector URnLineString::operator[](int32 Index) const
{ return (Points)[Index]->Vertex; }

FVector URnLineString::GetVertex(int32 Index) const
{
    return (*this)[Index];
}

TRnRef_T<URnPoint> URnLineString::GetPoint(int32 Index) const
{
    return (Points)[Index];
}

void URnLineString::SetPoint(int32 Index, const TRnRef_T<URnPoint>& Point)
{
    (Points)[Index] = Point;
}

FVector URnLineString::GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    float CurrentLength = 0.0f;
    OutStartIndex = 0;
    OutEndIndex = 0;

    for (int32 i = 0; i < Points.Num() - 1; ++i) {
        float SegmentLength = (GetVertex(i + 1) - GetVertex(i)).Size();
        if (CurrentLength + SegmentLength >= Offset) {
            OutStartIndex = i;
            OutEndIndex = i + 1;
            float T = (Offset - CurrentLength) / SegmentLength;
            return FMath::Lerp(GetVertex(i), GetVertex(i + 1), T);
        }
        CurrentLength += SegmentLength;
    }

    return Points.Last()->Vertex;
}

FVector URnLineString::GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    float CurrentLength = 0.0f;
    OutStartIndex = Points.Num() - 1;
    OutEndIndex = Points.Num() - 1;

    for (int32 i = Points.Num() - 1; i > 0; --i) {
        float SegmentLength = (GetVertex(i) - GetVertex(i-1)).Size();
        if (CurrentLength + SegmentLength >= Offset) {
            OutStartIndex = i;
            OutEndIndex = i - 1;
            float T = (Offset - CurrentLength) / SegmentLength;
            return FMath::Lerp(GetVertex(i), GetVertex(i-1), T);
        }
        CurrentLength += SegmentLength;
    }

    return GetVertex(0);
}

TArray<TTuple<float, FVector>> URnLineString::GetIntersectionBy2D(
    const FLineSegment3D& LineSegment,
    EAxisPlane Plane) const {
    TArray<TTuple<float, FVector>> Result;
    auto Edges = GetEdges();
    for(auto i = 0; i < Edges.Num(); ++i)
    {
        auto E = Edges[i];
        FVector P;
        float T1;
        float T2;
        if(E.TrySegmentIntersectionBy2D(LineSegment, Plane, -1.f, P, T1, T2))
        {
            auto V = E.Lerp(T1);
            Result.Add(MakeTuple(i + T2, V));
        }
    }

    return Result;
}
