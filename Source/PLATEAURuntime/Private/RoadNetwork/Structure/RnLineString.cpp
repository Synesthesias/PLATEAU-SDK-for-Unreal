#include "RoadNetwork/Structure/RnLineString.h"

#include <optional>

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Util/Vector2DEx.h"

RnLineString::RnLineString() {
}

RnLineString::RnLineString(int32 InitialSize) {
    Points = TSharedPtr<TArray<RnRef_t<RnPoint>>>();
    Points->SetNum(InitialSize);
}

RnLineString::RnLineString(const TSharedPtr<TArray<RnRef_t<RnPoint>>>& InPoints)
{
    Points = InPoints;
}


int32 RnLineString::Count() const
{ return Points->Num(); }

bool RnLineString::IsValid() const
{ return Points->Num() >= 2; }

void RnLineString::AddPointOrSkip(RnRef_t<RnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
    if (!Point) 
        return;

    if (Points->Num() > 0) {
        const float SqrDistanceThreshold = DistanceEpsilon < 0.0f ? -1.0f : DistanceEpsilon * DistanceEpsilon;
        if (RnPoint::Equals(Points->Last(), Point, SqrDistanceThreshold))
            return;
    }

    Points->Add(Point);
}

FVector RnLineString::GetEdgeNormal(int32 StartVertexIndex) const {
    const FVector P0 = (*this)[StartVertexIndex];
    const FVector P1 = (*this)[StartVertexIndex + 1];
    return FVector::CrossProduct(FVector::UpVector, P1 - P0).GetSafeNormal() * -1.0f;
}

RnRef_t<RnLineString> RnLineString::Clone(bool CloneVertex) const {
    auto NewLineString = RnNew<RnLineString>();

    if (CloneVertex) {
        for (const RnRef_t<RnPoint> Point : *Points) {
            NewLineString->Points->Add(Point->Clone());
        }
    }
    else {
        NewLineString->Points = Points;
    }

    return NewLineString;
}

float RnLineString::CalcLength() const {
    if (!IsValid()) return 0.0f;

    float Length = 0.0f;
    for (int32 i = 0; i < Points->Num() - 1; ++i) {
        Length += ((*Points)[i + 1]->Vertex - (*Points)[i]->Vertex).Size();
    }
    return Length;
}

TArray<RnRef_t<RnLineString>> RnLineString::Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector) {
    TArray<RnRef_t<RnLineString>> Result;
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

    for (int32 i = 0; i < Points->Num() - 1; i++) {
        const float SegmentLength = (GetVertex(i + 1) - GetVertex(i)).Size();
        const float NextLength = CurrentLength + SegmentLength;

        while (CurrentIndex < Rates.Num() &&
            CurrentLength <= Rates[CurrentIndex] * TotalLength &&
            Rates[CurrentIndex] * TotalLength < NextLength) {
            float T = (Rates[CurrentIndex] * TotalLength - CurrentLength) / SegmentLength;
            if (InsertNewPoint) {
                auto p = RnNew<RnPoint>(FMath::Lerp(GetVertex(i), GetVertex(i+1), T));
                Points->Insert(p, i + 1);
                i++;
            }
            SplitIndices.Add(i);
            CurrentIndex++;
        }
        CurrentLength = NextLength;
    }

    SplitIndices.Add(Points->Num() - 1);
    int32 StartIndex = 0;
    for (const int32 EndIndex : SplitIndices) {
        auto SplitPoints =  MakeShared<TArray<RnRef_t<RnPoint>>>();
        for (int32 i = StartIndex; i <= EndIndex; i++) {
            SplitPoints->Add(GetPoint(i));
        }
        Result.Add(Create(SplitPoints, false));
        StartIndex = EndIndex;
    }

    return Result;
}

TArray<RnRef_t<RnLineString>> RnLineString::SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint) const {
    TArray<RnRef_t<RnLineString>> Result;
    if (!IsValid() || Indices.Num() == 0) return Result;

    TArray<int32> SortedIndices = Indices;
    SortedIndices.Sort();

    int32 StartIndex = 0;
    for (int32 EndIndex : SortedIndices) {
        auto SplitPoints = MakeShared<TArray< RnRef_t<RnPoint>>>();
        for (int32 i = StartIndex; i <= EndIndex; ++i) {
            SplitPoints->Add(GetPoint(i));
        }

        if (SplitPoints->Num() >= 2) {
            RnRef_t<RnLineString> Split = Create(SplitPoints, false);
            Result.Add(Split);
        }
        StartIndex = EndIndex;
    }

    return Result;
}

void RnLineString::AddFrontPoint(RnRef_t<RnPoint> Point) {
    if (Point) {
        Points->Insert(Point, 0);
    }
}
void RnLineString::AddPointFrontOrSkip(RnRef_t<RnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
    if (!Point) return;

    if (Points->Num() > 0) {
        const float SqrDistanceThreshold = DistanceEpsilon < 0.0f ? -1.0f : DistanceEpsilon * DistanceEpsilon;
        if (RnPoint::Equals(GetPoint(0), Point, SqrDistanceThreshold)) {
            return;
        }
    }

    Points->Insert(Point, 0);
}

float RnLineString::CalcLength(float StartIndex, float EndIndex) const {
    if (!IsValid()) return 0.0f;

    const int32 StartIdx = FMath::FloorToInt(StartIndex);
    const int32 EndIdx = FMath::CeilToInt(EndIndex);

    float Length = 0.0f;
    for (int32 i = StartIdx; i < EndIdx && i < Points->Num() - 1; ++i) {
        Length += (GetVertex(i + 1) - GetVertex(i)).Size();
    }

    return Length;
}

float RnLineString::CalcTotalAngle2D() const {
    if (Points->Num() < 3) return 0.0f;

    float TotalAngle = 0.0f;
    std::optional<FVector2D> Last = std::nullopt;
    const auto Edges = FGeoGraphEx::GetEdges(*Points, false);
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

TArray<FLineSegment2D> RnLineString::GetEdges2D(EAxisPlane axis) const
{
    TArray<FLineSegment2D> Ret;
    const auto Edges = FGeoGraphEx::GetEdges(*Points, false);
    for (auto It = Edges.begin(); It != Edges.end(); ++It) 
    {
        const auto e = *It;
        Ret.Add(FLineSegment3D(e.P0->Vertex, e.P0->Vertex).To2D(axis));
    }
    return Ret;
}


TArray<FLineSegment3D> RnLineString::GetEdges() const
{
    TArray<FLineSegment3D> Ret;
    const auto Edges = FGeoGraphEx::GetEdges(*Points, false);
    for (auto It = Edges.begin(); It != Edges.end(); ++It) {
        const auto e = *It;
        Ret.Add(FLineSegment3D(e.P0->Vertex, e.P0->Vertex));
    }
    return Ret;
}

bool RnLineString::Contains(RnRef_t<RnPoint> Point) const {
    return Points->Contains(Point);
}

void RnLineString::GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    OutDistance = MAX_FLT;
    OutPointIndex = 0;

    for (int32 i = 0; i < Points->Num() - 1; ++i) {
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

float RnLineString::GetDistance2D(const RnRef_t<RnLineString> Other, EAxisPlane Plane) const {
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

FVector RnLineString::GetVertexNormal(int32 VertexIndex) const {
    if (Points->Num() <= 1) return FVector::ZeroVector;

    FVector Normal = FVector::ZeroVector;
    int32 Count = 0;

    if (VertexIndex > 0) {
        Normal += GetEdgeNormal(VertexIndex - 1);
        Count++;
    }

    if (VertexIndex < Points->Num() - 1) {
        Normal += GetEdgeNormal(VertexIndex);
        Count++;
    }

    return (Count > 0) ? Normal / Count : FVector::ZeroVector;
}

int32 RnLineString::ReplacePoint(RnRef_t<RnPoint> OldPoint, RnRef_t<RnPoint> NewPoint) {
    int32 ReplaceCount = 0;
    for (int32 i = 0; i < Points->Num(); i++) {
        if (GetPoint(i) == OldPoint) {
            (*Points)[i] = NewPoint;
            ReplaceCount++;
        }
    }
    return ReplaceCount;
}


RnRef_t<RnLineString> RnLineString::Create(const TSharedPtr<TArray<RnRef_t<RnPoint>>>& Vertices,
    bool RemoveDuplicate) {
    auto LineString = RnNew<RnLineString>();
    if (!RemoveDuplicate) {
        LineString->Points = Vertices;
        return LineString;
    }

    for (RnRef_t<RnPoint> Point : *Vertices) {
        LineString->AddPointOrSkip(Point);
    }

    return LineString;
}

RnRef_t<RnLineString> RnLineString::Create(const TArray<FVector>& Vertices, bool RemoveDuplicate) {
    auto Points = MakeShared<TArray<RnRef_t<RnPoint>>>();
    for (const FVector& Vertex : Vertices) 
    {
        auto P = RnNew<RnPoint>(Vertex);        
        Points->Add(P);
    }

    return Create(Points, RemoveDuplicate);
}

bool RnLineString::Equals(const RnRef_t<RnLineString> X, const RnRef_t<RnLineString> Y) {
    if (X == Y) return true;
    if (!X || !Y) return false;
    if (X->Points->Num() != Y->Points->Num()) return false;

    for (int32 i = 0; i < X->Points->Num(); i++) {
        if (!RnPoint::Equals(X->GetPoint(i), Y->GetPoint(i))) {
            return false;
        }
    }

    return true;
}

FVector RnLineString::operator[](int32 Index) const
{ return (*Points)[Index]->Vertex; }

FVector RnLineString::GetVertex(int32 Index) const
{
    return (*this)[Index];
}

RnRef_t<RnPoint> RnLineString::GetPoint(int32 Index) const
{
    return (*Points)[Index];
}

void RnLineString::SetPoint(int32 Index, const RnRef_t<RnPoint>& Point)
{
    (*Points)[Index] = Point;
}

FVector RnLineString::GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    float CurrentLength = 0.0f;
    OutStartIndex = 0;
    OutEndIndex = 0;

    for (int32 i = 0; i < Points->Num() - 1; ++i) {
        float SegmentLength = (GetVertex(i + 1) - GetVertex(i)).Size();
        if (CurrentLength + SegmentLength >= Offset) {
            OutStartIndex = i;
            OutEndIndex = i + 1;
            float T = (Offset - CurrentLength) / SegmentLength;
            return FMath::Lerp(GetVertex(i), GetVertex(i + 1), T);
        }
        CurrentLength += SegmentLength;
    }

    return Points->Last()->Vertex;
}

FVector RnLineString::GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    float CurrentLength = 0.0f;
    OutStartIndex = Points->Num() - 1;
    OutEndIndex = Points->Num() - 1;

    for (int32 i = Points->Num() - 1; i > 0; --i) {
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

TArray<TTuple<float, FVector>> RnLineString::GetIntersectionBy2D(
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
