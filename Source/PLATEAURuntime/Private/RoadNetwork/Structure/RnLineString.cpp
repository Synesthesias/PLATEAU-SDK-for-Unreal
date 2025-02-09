#include "RoadNetwork/Structure/RnLineString.h"

#include <optional>

#include "RoadNetwork/GeoGraph/GeoGraph2d.h"
#include "RoadNetwork/GeoGraph/GeoGraphEx.h"
#include "RoadNetwork/Util/PLATEAUVector2DEx.h"

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
        Length += (Points[i + 1]->Vertex - Points[i]->Vertex).Size();
    }
    return Length;
}

TArray<TRnRef_T<URnLineString>> URnLineString::Split(int32 Num, bool InsertNewPoint, TFunction<float(int32)> RateSelector) {
    TArray<TRnRef_T<URnLineString>> Result;
    if (!IsValid() || Num <= 0) return Result;

    if (!RateSelector)
        RateSelector = [Num](int32) { return 1.f / Num; };

    TArray<TRnRef_T<URnLineString>> Ret;

    const float TotalLength = CalcLength();
    float len = 0.0f;
    TArray<TRnRef_T<URnPoint>> SubVertices = { Points[0]};

    auto GetLength = [&](int32 Index) -> float {
        return TotalLength * RateSelector(Index);
        };
    for (int32 i = 1; i < Points.Num(); i++) 
    {
        auto p0 = SubVertices.Last();
        auto p1 = Points[i];
        auto l = (p1->Vertex - p0->Vertex).Length();
        len += l;

        auto length = GetLength(Ret.Num());
        // lenがlengthを超えたら分割線分を追加
        // ただし、最後の線は全部追加する
        while (len >= length && l >= FGeoGraph2D::Eps && Ret.Num() < (Num - 1)) {
            // #TODO : マジックナンバー
            //       : 分割点が隣り合う点とこれ以下の場合は新規で作らず使いまわす
            auto mergeLength = FMath::Min( FPLATEAURnDef::Meter2Unit * 0.1f, length * 0.5f);
            auto threshold = mergeLength * mergeLength;
            auto f = 1.f - (len - length) / l;
            auto end = RnNew<URnPoint>(FMath::Lerp(p0->Vertex, p1->Vertex, f));

            // もし,p0/p1とほぼ同じ点ならそっちを使う
            // ただし、その結果subVerticesが線分にならない場合は無視する
            if (SubVertices.Num() > 1) {
                if ((p1->Vertex - end->Vertex).SquaredLength() < threshold) {
                    end = p1;
                }
                else if ((p0->Vertex - end->Vertex).SquaredLength() < threshold) {
                    end = p0;
                }
            }

            // 同一頂点が複数あった場合は無視する
            if (f >= FLT_EPSILON) {
                SubVertices.Add(end);
                // 自分自身にも追加する場合
                if (InsertNewPoint && p1 != end && p0 != end) {
                    Points.Insert(end, i);
                    i += 1;
                }
            }

            Ret.Add( RnNew<URnLineString>(SubVertices));
            SubVertices = { end };
            len -= length;
            // 次の長さを更新
            length = GetLength(Ret.Num());
        }
        if (SubVertices.IsEmpty() == false && SubVertices.Last() != p1)
            SubVertices.Add(p1);
    }

    // 最後の要素は無条件で返す
    if (Ret.Num() < Num && SubVertices.IsEmpty() == false) {
        if (SubVertices.Last() != Points.Last())
            SubVertices.Add(Points.Last());
        if (SubVertices.Num() > 1)
            Ret.Add(RnNew<URnLineString>(SubVertices));
    }
    return Ret;
}

bool URnLineString::SplitByIndex(float Index, URnLineString*& OutFront, URnLineString*& OutBack, TFunction<URnPoint*(FVector)> CreatePoint) const
{
    auto IsInt = FMath::Abs(Index - FMath::RoundToFloat(Index)) < KINDA_SMALL_NUMBER;

    auto I = IsInt ? FMath::RoundToFloat(Index) : (int)Index;

    // points = [v0, v1, v2, v3]の時
    // index = 1で区切るときは, front = [v0, v1], back = [v1, v2, v3]
    // -> i = 1, frontはTake(1), backはskip(2)にして、v1をお互いに追加
    // index = 1.5で区切るときは,front = [v0, v1, v1.5], back = [v1.5, v2, v3]
    // -> i = 1, frontはTake(2), backはskip(2)にして、v1.5を追加

    TArray<URnPoint*> FrontPoints;
    TArray<URnPoint*> BackPoints;
    for(auto i = 0; i <= FMath::Min(I, Count() - 1); ++i)
        FrontPoints.Add(Points[i]);

    // 整数の時はfrontの最後をbackの最初に追加
    if (IsInt) {
        BackPoints.Add(Points[I]);
    }
    else
    {
        // 少数の時は中間点をfontの最後とbackの最初に追加
        auto V = FMath::Lerp(Points[I]->Vertex, Points[I + 1]->Vertex, 0.5f);
        auto MidPoint = CreatePoint(V);
        FrontPoints.Add(MidPoint);
        BackPoints.Add(MidPoint);
    }

    for (auto i = I + 1; i < Count(); ++i)
        BackPoints.Add(Points[i]);


    TArray<TRnRef_T<URnLineString>> Result;
    OutFront = RnNew<URnLineString>(FrontPoints);
    OutBack = RnNew<URnLineString>(BackPoints);
    return true;
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
        auto Dir = FLineSegment3D(e.P0->Vertex, e.P1->Vertex).To2D(FPLATEAURnDef::Plane).GetDirection();
        if(Last.has_value())
        {
            TotalAngle += FPLATEAUVector2DEx::Angle((*Last), Dir);
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
        Ret.Add(FLineSegment3D(e.P0->Vertex, e.P1->Vertex));
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
    const FLineSegment3D& Ray,
    EAxisPlane Plane) const {
    TArray<TTuple<float, FVector>> Result;
    auto Edges = GetEdges();
    for(auto i = 0; i < Edges.Num(); ++i)
    {
        auto E = Edges[i];
        FVector P;
        float T1;
        float T2;
        if(E.TrySegmentIntersectionBy2D(Ray, Plane, -1.f, P, T1, T2))
        {
            auto V = E.Lerp(T1);
            Result.Add(MakeTuple(i + T1, V));
        }
    }

    return Result;
}

TArray<TTuple<float, FVector>> URnLineString::GetIntersectionBy2D(const FRay& Ray, EAxisPlane Plane) const
{
    TArray<TTuple<float, FVector>> Result;
    auto Edges = GetEdges();
    for (auto i = 0; i < Edges.Num(); ++i) {
        auto E = Edges[i];
        FVector P;
        float T1;
        float T2;
        if (E.TryLineIntersectionBy2D(Ray.Origin, Ray.Direction, Plane, -1.f, P, T1, T2)) {
            Result.Add(MakeTuple(i + T1, P));
        }
    }

    return Result;
}

bool URnLineString::TryGetNearestIntersectionBy2D(const FRay& Ray, TTuple<float, FVector>& Res,
                                                  EAxisPlane Plane) const {
    auto ret = GetIntersectionBy2D(Ray, Plane);
    if (ret.IsEmpty()) {
        return false;
    }
    return FPLATEAURnLinq::TryFindMinElement<TTuple<float, FVector>>(
        ret,
        [&](const TTuple<float, FVector>& X, const TTuple<float, FVector>& Y) {
            auto A = (X.Value - Ray.Origin).SizeSquared();
            auto B = (Y.Value - Ray.Origin).SizeSquared();
            return A < B;
        }
    , Res);
}

TOptional<float> URnLineString::CalcProximityScore(const URnLineString* Other) const
{
    if (IsValid() == false || !Other || !Other->IsValid())
        return NullOpt;

    return FPLATEAURnLinq::Average<URnPoint*, float>(Points, [&](const TRnRef_T<URnPoint>& V) {
        FVector Inter;
        float Index;
        float Distance;
        Other->GetNearestPoint(V->Vertex, Inter, Index, Distance);
        return Distance;
        });
}



