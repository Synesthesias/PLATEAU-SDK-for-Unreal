#include "RoadNetwork/Structure/RnWay.h"

#include "EditorCategoryUtils.h"

RnWay::RnWay()
    : IsReversed(false)
    , IsReverseNormal(false) {
}

RnWay::RnWay(const RnRef_t<RnLineString>& InLineString, bool bInIsReversed, bool bInIsReverseNormal)
    : LineString(InLineString)
    , IsReversed(bInIsReversed)
    , IsReverseNormal(bInIsReverseNormal) {
}

int32 RnWay::Count() const
{ return LineString ? LineString->Points->Num() : 0; }

bool RnWay::IsValid() const
{ return LineString ? LineString->IsValid() : false; }

RnRef_t<RnPoint> RnWay::GetPoint(int32 Index) const {
    int32 RawIndex = ToRawIndex(Index, true);
    return LineString->GetPoint(RawIndex);
}

RnRef_t<RnPoint> RnWay::SetPoint(int32 Index, const RnRef_t<RnPoint>& Point) {
    int32 RawIndex = ToRawIndex(Index, true);
    auto Ret = LineString->GetPoint(RawIndex);
    LineString->SetPoint(RawIndex, Point);
    return Ret;
}

FVector RnWay::GetVertex(int32 Index) const
{
    return GetPoint(Index)->Vertex;
}

FVector RnWay::operator[](int32 Index) const {
    return GetVertex(Index);
}

RnRef_t<RnWay> RnWay::ReversedWay() const {
    return RnNew<RnWay>(LineString, !IsReversed, !IsReverseNormal);
}

void RnWay::Reverse(bool KeepNormalDir) {
    IsReversed = !IsReversed;
    if (KeepNormalDir) {
        IsReverseNormal = !IsReverseNormal;
    }
}

int32 RnWay::ToRawIndex(int32 Index, bool AllowMinus) const {
    if (AllowMinus && Index < 0) {
        Index = Count() + Index;
    }
    return IsReversed ? Count() - 1 - Index : Index;
}

int32 RnWay::SwitchIndex(int32 Index) const {
    return IsReversed ? Count() - 1 - Index : Index;
}

float RnWay::SwitchIndex(float Index) const {
    return IsReversed ? Count() - 1 - Index : Index;
}

// Add implementations:
TArray<FLineSegment2D> RnWay::GetEdges2D() const {
    TArray<FLineSegment2D> Edges;
    for (int32 i = 0; i < Count() - 1; i++) {
        Edges.Add(FLineSegment3D(GetPoint(i)->Vertex, GetPoint(i+1)->Vertex).To2D(FRnDef::Plane));
    }
    return Edges;
}

int32 RnWay::FindPoint(const RnRef_t<RnPoint>& Point) const {
    return LineString->Points->IndexOfByPredicate([&](const RnRef_t<RnPoint>& P) { return P == Point; });
}

int32 RnWay::FindPointIndex(const RnRef_t<RnPoint>& Point) const {
    int32 Index = LineString->Points->IndexOfByKey(Point);
    return Index < 0 ? Index : SwitchIndex(Index);
}

void RnWay::GetNearestPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    OutNearest = FVector::ZeroVector;
    LineString->GetNearestPoint(Pos, OutNearest, OutPointIndex, OutDistance);
    OutPointIndex = SwitchIndex(OutPointIndex);
}

bool RnWay::IsValidOrDefault() const {
    return IsValid();
}

float RnWay::CalcLength() const {
    return LineString ? LineString->CalcLength() : 0.0f;
}

float RnWay::CalcLength(float StartIndex, float EndIndex) const {
    if (IsReversed) {
        return LineString->CalcLength(SwitchIndex(EndIndex), SwitchIndex(StartIndex));
    }
    return LineString->CalcLength(StartIndex, EndIndex);
}

void RnWay::AppendBack2LineString(const RnRef_t<RnWay>& Back) {
    if (!Back || Back->LineString == LineString) return;

    if (IsReversed) {
        for (const auto& P : *(Back->LineString->Points)) {
            LineString->AddPointFrontOrSkip(P);
        }
    }
    else {
        for (const auto& P : *(Back->LineString->Points)) {
            LineString->AddPointOrSkip(P);
        }
    }
}

void RnWay::AppendFront2LineString(const RnRef_t<RnWay>& Front) {
    if (!Front || Front->LineString == LineString) return;

    if (IsReversed) {
        for (int32 i = 0; i < Front->Count(); ++i) {
            LineString->AddPointOrSkip(Front->GetPoint(Front->Count() - 1 - i));
        }
    }
    else {
        for (int32 i = 0; i < Front->Count(); ++i) {
            LineString->AddPointFrontOrSkip(Front->GetPoint(Front->Count() - 1 - i));
        }
    }
}

bool RnWay::IsOutSide(const FVector& V, FVector& OutNearest, float& OutDistance) const {
    float PointIndex;
    GetNearestPoint(V, OutNearest, PointIndex, OutDistance);

    if (!IsValidOrDefault()) return false;

    int32 Start = FMath::Clamp(static_cast<int32>(PointIndex), 0, Count() - 2);
    int32 End = FMath::Clamp(FMath::CeilToInt(PointIndex - 1), 0, Count() - 2);

    TSet<int32> IndexSet = { Start, End };
    FVector Delta = V - OutNearest;
    for(auto I : IndexSet)
    {
        if (FVector2D::DotProduct( FRnDef::To2D(GetEdgeNormal(I)), FRnDef::To2D(Delta)) >= 0.0f)
            return true;
    }
    return false;
}

void RnWay::MoveLerpAlongNormal(const FVector& StartOffset, const FVector& EndOffset) {
    if (!IsValid()) return;

    if (Count() == 2) {
        GetPoint(0)->Vertex += StartOffset;
        GetPoint(1)->Vertex += EndOffset;
        return;
    }

    TArray<FVector> PointOffsets;
    PointOffsets.SetNum(Count());
    PointOffsets[0] = StartOffset;
    PointOffsets.Last() = EndOffset;

    float TotalLength = CalcLength();
    float CurrentLength = 0.0f;

    for (int32 i = 1; i < Count() - 1; ++i) {
        CurrentLength += ((*this)[i] - (*this)[i - 1]).Size();
        float T = CurrentLength / TotalLength;

        FVector Normal = GetVertexNormal(i);
        float StartLen = StartOffset.Size();
        float EndLen = EndOffset.Size();
        float LerpLen = FMath::Lerp(StartLen, EndLen, T);

        PointOffsets[i] = Normal * LerpLen;
    }

    for (int32 i = 0; i < Count(); ++i) {
        GetPoint(i)->Vertex += PointOffsets[i];
    }
}

FVector RnWay::GetAdvancedPointFromFront(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    if (IsReversed) {
        auto Result = LineString->GetAdvancedPointFromBack(Offset, OutStartIndex, OutEndIndex);
        OutStartIndex = SwitchIndex(OutStartIndex);
        OutEndIndex = SwitchIndex(OutEndIndex);
        return Result;
    }

    auto Result = LineString->GetAdvancedPointFromFront(Offset, OutStartIndex, OutEndIndex);
    OutStartIndex = SwitchIndex(OutStartIndex);
    return Result;
}

FVector RnWay::GetAdvancedPointFromBack(float Offset, int32& OutStartIndex, int32& OutEndIndex) const {
    if (IsReversed) {
        auto Result = LineString->GetAdvancedPointFromFront(Offset, OutStartIndex, OutEndIndex);
        OutStartIndex = SwitchIndex(OutStartIndex);
        OutEndIndex = SwitchIndex(OutEndIndex);
        return Result;
    }

    auto Result = LineString->GetAdvancedPointFromBack(Offset, OutStartIndex, OutEndIndex);
    OutStartIndex = SwitchIndex(OutStartIndex);
    OutEndIndex = SwitchIndex(OutEndIndex);
    return Result;
}

FVector RnWay::GetAdvancedPoint(float Offset, bool Reverse, int32& OutStartIndex, int32& OutEndIndex) const {
    return Reverse ?
        GetAdvancedPointFromBack(Offset, OutStartIndex, OutEndIndex) :
        GetAdvancedPointFromFront(Offset, OutStartIndex, OutEndIndex);
}

FVector RnWay::GetAdvancedPoint(float Offset, bool Reverse) const {
    int32 StartIndex, EndIndex;
    return GetAdvancedPoint(Offset, Reverse, StartIndex, EndIndex);
}

float RnWay::GetDistance2D(const RnRef_t<RnWay>& Other, EAxisPlane Plane) const {
    return LineString ? LineString->GetDistance2D(Other->LineString, Plane) : MAX_FLT;
}

FVector RnWay::GetEdgeNormal(int32 StartVertexIndex) const {
    int32 Index = ToRawIndex(StartVertexIndex);
    if (IsReversed) {
        Index -= 1;
    }
    FVector Normal = LineString->GetEdgeNormal(Index);
    if (IsReversed != IsReverseNormal) {
        Normal *= -1.0f;
    }
    return Normal;
}

FVector RnWay::GetVertexNormal(int32 VertexIndex) const {
    if (Count() <= 1) {
        return FVector::ZeroVector;
    }

    FVector Normal = LineString->GetVertexNormal(ToRawIndex(VertexIndex));
    if (IsReversed != IsReverseNormal) {
        Normal *= -1.0f;
    }
    return Normal;
}

// Add implementations:
FVector RnWay::GetLerpPoint(float P) const {
    FVector MidPoint;
    GetLerpPoint(P, MidPoint);
    return MidPoint;
}

float RnWay::GetLerpPoint(float P, FVector& OutMidPoint) const {
    float TotalLength = CalcLength();
    float TargetLength = TotalLength * P;
    float CurrentLength = 0.0f;

    for (int32 i = 0; i < Count() - 1; ++i) {
        FVector Start = (*this)[i];
        FVector End = (*this)[i + 1];
        float SegmentLength = (End - Start).Size();

        if (CurrentLength + SegmentLength >= TargetLength) {
            float T = (TargetLength - CurrentLength) / SegmentLength;
            OutMidPoint = FMath::Lerp(Start, End, T);
            return static_cast<float>(i) + T;
        }
        CurrentLength += SegmentLength;
    }

    OutMidPoint = (*this)[Count() - 1];
    return static_cast<float>(Count() - 1);
}

void RnWay::MoveAlongNormal(float Offset) {
    if (!IsValid()) return;

    // Special handling for 2 vertices
    if (Count() == 2) {
        FVector Normal = GetEdgeNormal(0);
        for (auto Point : GetPoints()) {
            Point->Vertex += Normal * Offset;
        }
        return;
    }

    int32 Index = 0;
    TArray<FVector> EdgeNormals = { GetEdgeNormal(0), GetEdgeNormal(FMath::Min(Count() - 1, 1)) };
    TArray<FVector> VertexNormals = { GetVertexNormal(0), GetVertexNormal(1) };
    float Delta = Offset;

    for (int32 i = 0; i < Count(); ++i) {
        FVector En0 = EdgeNormals[Index];
        FVector En1 = EdgeNormals[(Index + 1) & 1];
        FVector Vn = VertexNormals[Index];

        float M = FVector::DotProduct(Vn, En0);
        float D = Delta;
        bool bIsZero = FMath::Abs(M) < 1e-5f;
        if (!bIsZero) {
            D /= M;
        }

        FVector O = Vn * D;
        if (i < Count() - 1) {
            VertexNormals[Index] = GetVertexNormal(i + 1);
            EdgeNormals[Index] = GetEdgeNormal(FMath::Min(Count() - 2, i + 1));
            Index = (Index + 1) & 1;
        }

        GetPoint(i)->Vertex += O;
        Delta = D * FVector::DotProduct(Vn, En1);
    }
}

void RnWay::Move(const FVector& Offset) {
    for (int32 i = 0; i < Count(); ++i) {
        GetPoint(i)->Vertex += Offset;
    }
}

bool RnWay::IsSameLineReference(const RnRef_t<RnWay>& Other) const {
    if (!Other) return false;
    return LineString == Other->LineString;
}

bool RnWay::IsSameLineSequence(const RnRef_t<RnWay>& Other) const {
    if (!Other) return false;
    if (!LineString || !Other->LineString) return false;
    if (Count() != Other->Count()) return false;

    for (int32 i = 0; i < Count(); ++i) 
    {
        constexpr float Threshold = 0.001f;
        const auto P1 = GetPoint(i)->Vertex;
        const auto P2 = Other->GetPoint(i)->Vertex;
        if (FMath::Abs(P1.X - P2.X) > Threshold) return false;
        if (FMath::Abs(P1.Y - P2.Y) > Threshold) return false;
        if (FMath::Abs(P1.Z - P2.Z) > Threshold) return false;
    }
    return true;
}
