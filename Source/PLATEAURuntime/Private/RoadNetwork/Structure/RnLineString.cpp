#include "RoadNetwork/Structure/RnLineString.h"

#include <optional>

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Util/Vector2DEx.h"

RnLineString::RnLineString() {
}

RnLineString::RnLineString(int32 InitialSize) {
    Points = std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>();
    Points->SetNum(InitialSize);
}

RnLineString::RnLineString(const std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>& InPoints)
{
    Points = InPoints;
}


int32 RnLineString::Count() const
{ return Points->Num(); }

bool RnLineString::IsValid() const
{ return Points->Num() >= 2; }

void RnLineString::AddPointOrSkip(std::shared_ptr<RnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
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

std::shared_ptr<RnLineString> RnLineString::Clone(bool CloneVertex) const {
    auto NewLineString = std::make_shared<RnLineString>();

    if (CloneVertex) {
        for (const std::shared_ptr<RnPoint> Point : *Points) {
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

TArray<std::shared_ptr<RnLineString>> RnLineString::Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector) {
    TArray<std::shared_ptr<RnLineString>> Result;
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
        const float SegmentLength = (Get(i + 1) - Get(i)).Size();
        const float NextLength = CurrentLength + SegmentLength;

        while (CurrentIndex < Rates.Num() &&
            CurrentLength <= Rates[CurrentIndex] * TotalLength &&
            Rates[CurrentIndex] * TotalLength < NextLength) {
            float T = (Rates[CurrentIndex] * TotalLength - CurrentLength) / SegmentLength;
            if (InsertNewPoint) {
                auto p = std::make_shared<RnPoint>(FMath::Lerp(Get(i), Get(i+1), T));
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
        auto SplitPoints = std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>();
        for (int32 i = StartIndex; i <= EndIndex; i++) {
            SplitPoints->Add(GetPoint(i));
        }
        Result.Add(Create(SplitPoints, false));
        StartIndex = EndIndex;
    }

    return Result;
}

TArray<std::shared_ptr<RnLineString>> RnLineString::SplitByIndex(const TArray<int32>& Indices, bool InsertNewPoint) const {
    TArray<std::shared_ptr<RnLineString>> Result;
    if (!IsValid() || Indices.Num() == 0) return Result;

    TArray<int32> SortedIndices = Indices;
    SortedIndices.Sort();

    int32 StartIndex = 0;
    for (int32 EndIndex : SortedIndices) {
        auto SplitPoints = std::make_shared<TArray< std::shared_ptr<RnPoint>>>();
        for (int32 i = StartIndex; i <= EndIndex; ++i) {
            SplitPoints->Add(GetPoint(i));
        }

        if (SplitPoints->Num() >= 2) {
            std::shared_ptr<RnLineString> Split = Create(SplitPoints, false);
            Result.Add(Split);
        }
        StartIndex = EndIndex;
    }

    return Result;
}

void RnLineString::AddFrontPoint(std::shared_ptr<RnPoint> Point) {
    if (Point) {
        Points->Insert(Point, 0);
    }
}
void RnLineString::AddPointFrontOrSkip(std::shared_ptr<RnPoint> Point, float DistanceEpsilon, float DegEpsilon, float MidPointTolerance) {
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
        Length += (Get(i + 1) - Get(i)).Size();
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

bool RnLineString::Contains(std::shared_ptr<RnPoint> Point) const {
    return Points->Contains(Point);
}

void RnLineString::GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    OutDistance = MAX_FLT;
    OutPointIndex = 0;

    for (int32 i = 0; i < Points->Num() - 1; ++i) {
        const FVector Start = Get(i);
        const FVector End = Get(i+1);
        const FVector ProjectedPoint = FMath::ClosestPointOnSegment(Pos, Start, End);

        const float Distance = (Pos - ProjectedPoint).Size();
        if (Distance < OutDistance) {
            OutDistance = Distance;
            OutNearest = ProjectedPoint;
            OutPointIndex = i + (ProjectedPoint - Start).Size() / (End - Start).Size();
        }
    }
}

float RnLineString::GetDistance2D(const std::shared_ptr<RnLineString> Other, EAxisPlane Plane) const {
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

int32 RnLineString::ReplacePoint(std::shared_ptr<RnPoint> OldPoint, std::shared_ptr<RnPoint> NewPoint) {
    int32 ReplaceCount = 0;
    for (int32 i = 0; i < Points->Num(); i++) {
        if (GetPoint(i) == OldPoint) {
            (*Points)[i] = NewPoint;
            ReplaceCount++;
        }
    }
    return ReplaceCount;
}


std::shared_ptr<RnLineString> RnLineString::Create(const std::shared_ptr<TArray<std::shared_ptr<RnPoint>>>& Vertices,
    bool RemoveDuplicate) {
    auto LineString = std::make_shared<RnLineString>();
    if (!RemoveDuplicate) {
        LineString->Points = Vertices;
        return LineString;
    }

    for (std::shared_ptr<RnPoint> Point : *Vertices) {
        LineString->AddPointOrSkip(Point);
    }

    return LineString;
}

std::shared_ptr<RnLineString> RnLineString::Create(const TArray<FVector>& Vertices, bool RemoveDuplicate) {
    auto Points = std::make_shared<TArray<std::shared_ptr<RnPoint>>>();
    for (const FVector& Vertex : Vertices) 
    {
        auto P = std::make_shared<RnPoint>(Vertex);        
        Points->Add(P);
    }

    return Create(Points, RemoveDuplicate);
}

bool RnLineString::Equals(const std::shared_ptr<RnLineString> X, const std::shared_ptr<RnLineString> Y) {
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

FVector RnLineString::Get(int32 Index) const
{
    return (*this)[Index];
}

std::shared_ptr<RnPoint> RnLineString::GetPoint(int32 Index) const
{
    return (*Points)[Index];
}
