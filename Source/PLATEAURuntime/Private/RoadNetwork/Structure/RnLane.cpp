#include "RoadNetwork/Structure/RnLane.h"

#include "RoadNetwork/Util/PLATEAUVectorEx.h"

URnLane::URnLane()
    : bIsReverse(false) {
}

URnLane::URnLane(const TRnRef_T<URnWay>& InLeftWay, const TRnRef_T<URnWay>& InRightWay,
    const TRnRef_T<URnWay>& InPrevBorder, const TRnRef_T<URnWay>& InNextBorder)
    : bIsReverse(false) {
    Init(InLeftWay, InRightWay, InPrevBorder, InNextBorder);

}

void URnLane::Init()
{}

void URnLane::Init(const TRnRef_T<URnWay>& InLeftWay, const TRnRef_T<URnWay>& InRightWay,
                   const TRnRef_T<URnWay>& InPrevBorder, const TRnRef_T<URnWay>& InNextBorder)
{
    this->LeftWay    = InLeftWay;
    this->RightWay   = InRightWay;
    this->PrevBorder = InPrevBorder;
    this->NextBorder = InNextBorder;
}

TArray<TRnRef_T<URnWay>> URnLane::GetBothWays() const {
    TArray<TRnRef_T<URnWay>> Ways;
    if (LeftWay) Ways.Add(LeftWay);
    if (RightWay) Ways.Add(RightWay);
    return Ways;
}

TArray<TRnRef_T<URnWay>> URnLane::GetAllBorders() const {
    TArray<TRnRef_T<URnWay>> Borders;
    if (PrevBorder) Borders.Add(PrevBorder);
    if (NextBorder) Borders.Add(NextBorder);
    return Borders;
}

TArray<TRnRef_T<URnWay>> URnLane::GetAllWays() const {
    TArray<TRnRef_T<URnWay>> Ways = GetBothWays();
    Ways.Append(GetAllBorders());
    return Ways;
}

TRnRef_T<URnRoad> URnLane::GetParent() const
{
    return RnFrom(Parent);
}

void URnLane::SetParent(TRnRef_T<URnRoad> InParent)
{
    Parent = InParent;
}

TRnRef_T<URnWay> URnLane::GetLeftWay() const
{ return LeftWay; }

TRnRef_T<URnWay> URnLane::GetRightWay() const
{ return RightWay; }

TRnRef_T<URnWay> URnLane::GetPrevBorder() const
{ return PrevBorder; }

TRnRef_T<URnWay> URnLane::GetNextBorder() const
{ return NextBorder; }

bool URnLane::IsValidWay() const {
    return LeftWay && RightWay && LeftWay->IsValid() && RightWay->IsValid();
}

bool URnLane::IsBothConnectedLane() const {
    return GetPrevBorder() && GetNextBorder();
}

bool URnLane::HasBothBorder() const {
    return GetPrevBorder() && GetNextBorder();
}

bool URnLane::IsEmptyLane() const {
    return !GetLeftWay() && !GetRightWay() && HasBothBorder();
}

bool URnLane::IsMedianLane() const {
    return GetParent() ? GetParent()->IsMedianLane(RnFrom(this)) : false;
}
TOptional<EPLATEAURnLaneBorderDir> URnLane::GetBorderDir(EPLATEAURnLaneBorderType Type) const
{
    auto Border = GetBorder(Type);
    if (!Border)
        return NullOpt;
    if (!IsValidWay())
        return NullOpt;

    // とりあえず重なるかで判定
    // borderの0番目の点がLeftWayの0番目の点と同じならLeft2Right
    if(Border->GetPoint(0) == LeftWay->GetPoint(0))
        return EPLATEAURnLaneBorderDir::Left2Right;

    if (Border->GetPoint(0) == RightWay->GetPoint(0))
        return EPLATEAURnLaneBorderDir::Right2Left;

    if (!Border->IsValid())
        return NullOpt;

    auto D = Border->GetPoint(1)->Vertex - Border->GetPoint(0)->Vertex;
    auto Index = Type == EPLATEAURnLaneBorderType::Prev ? 0 : 1;
    auto D2 = RightWay->GetPoint(Index)->Vertex - LeftWay->GetPoint(Index)->Vertex;
    if (FVector2d::DotProduct(FPLATEAURnDef::To2D(D), FPLATEAURnDef::To2D(D2)) > 0.f)
        return EPLATEAURnLaneBorderDir::Left2Right;

    return EPLATEAURnLaneBorderDir::Right2Left;
}

TRnRef_T<URnWay> URnLane::GetBorder(EPLATEAURnLaneBorderType Type) const {
    return Type == EPLATEAURnLaneBorderType::Prev ? PrevBorder : NextBorder;
}

void URnLane::SetBorder(EPLATEAURnLaneBorderType Type, const TRnRef_T<URnWay>& Border) {
    if (Type == EPLATEAURnLaneBorderType::Prev)
        PrevBorder = Border;
    else
        NextBorder = Border;
}

TRnRef_T<URnWay> URnLane::GetSideWay(EPLATEAURnDir Dir) const {
    return Dir == EPLATEAURnDir::Left ? LeftWay : RightWay;
}

void URnLane::SetSideWay(EPLATEAURnDir Dir, const TRnRef_T<URnWay>& Way) {
    if (Dir == EPLATEAURnDir::Left)
        LeftWay = Way;
    else
        RightWay = Way;
}

float URnLane::CalcWidth() const {
    return FMath::Min(CalcPrevBorderWidth(), CalcNextBorderWidth());
}
float URnLane::CalcPrevBorderWidth() const {
    return FRnWayEx::CalcLengthOrDefault(PrevBorder);
}

/// <summary>
/// NextBorderの長さを返す
/// </summary>
/// <param name="self"></param>
/// <returns></returns>
float URnLane::CalcNextBorderWidth() const {
    return FRnWayEx::CalcLengthOrDefault(NextBorder);
}
float URnLane::CalcMinWidth() const
{
    if (!FRnWayEx::IsValidWayOrDefault(LeftWay))
        return 0.f;
    if (!FRnWayEx::IsValidWayOrDefault(RightWay))
        return 0.f;

    float MinW = MAX_FLT;
    for(auto V : LeftWay->GetVertices())
    {
        FVector OutNearest;
        float OutPointIndex;
        float OutDistance;
        RightWay->GetNearestPoint(V, OutNearest, OutPointIndex, OutDistance);
        MinW = FMath::Min(MinW, OutDistance);
    }
    for (auto V : RightWay->GetVertices()) {
        FVector OutNearest;
        float OutPointIndex;
        float OutDistance;
        LeftWay->GetNearestPoint(V, OutNearest, OutPointIndex, OutDistance);
        MinW = FMath::Min(MinW, OutDistance);
    }
    return MinW;
}

void URnLane::Reverse() {
    bIsReverse = !bIsReverse;
    Swap(PrevBorder, NextBorder);
    Swap(LeftWay, RightWay);
    for (auto Way : GetAllWays())
        Way->Reverse(true);
}

void URnLane::AlignBorder(EPLATEAURnLaneBorderDir borderDir)
{
    AlignBorder(EPLATEAURnLaneBorderType::Prev, borderDir);
    AlignBorder(EPLATEAURnLaneBorderType::Next, borderDir);
}

void URnLane::BuildCenterWay() {
    if (!IsValidWay()) return;

    auto NewCenterWay = RnNew<URnWay>();
    NewCenterWay->LineString = RnNew<URnLineString>();

    for (int32 i = 0; i < LeftWay->Count(); ++i) {
        auto NewPoint = RnNew<URnPoint>();
        NewPoint->Vertex = (LeftWay->GetPoint(i)->Vertex + RightWay->GetPoint(i)->Vertex) * 0.5f;
        NewCenterWay->LineString->GetPoints().Add(NewPoint);
    }

    CenterWay = NewCenterWay;
}

TRnRef_T<URnWay> URnLane::GetCenterWay() {
    if (!CenterWay) BuildCenterWay();
    return CenterWay;
}

void URnLane::GetNearestCenterPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    if (!CenterWay) {
        const_cast<URnLane*>(this)->BuildCenterWay();
    }
    CenterWay->GetNearestPoint(Pos, OutNearest, OutPointIndex, OutDistance);
}

float URnLane::GetCenterLength() const {
    return CenterWay ? CenterWay->CalcLength() : 0.0f;
}

float URnLane::GetDistanceFrom(const FVector& Point) const {
    if (!IsValidWay()) return MAX_FLT;

    FVector Nearest;
    float PointIndex, Distance;
    GetNearestCenterPoint(Point, Nearest, PointIndex, Distance);
    return Distance;
}

bool URnLane::IsInside(const FVector& Point) const {
    if (!IsValidWay()) return false;

    FVector Nearest;
    float Distance;
    return !LeftWay->IsOutSide(Point, Nearest, Distance) &&
        !RightWay->IsOutSide(Point, Nearest, Distance);
}


FVector URnLane::GetCentralVertex() const {
    TArray<FVector> Points;
    if (GetLeftWay())
        Points.Add(GetLeftWay()->GetLerpPoint(0.5f));
    if (GetRightWay())
        Points.Add(GetRightWay()->GetLerpPoint(0.5f));
    return FPLATEAUVectorEx::Centroid(Points);
}



TRnRef_T<URnLane> URnLane::Clone() const {
    auto NewLane = RnNew<URnLane>();
    NewLane->LeftWay = LeftWay ? LeftWay->Clone(true) : nullptr;
    NewLane->RightWay = RightWay ? RightWay->Clone(true) : nullptr;
    NewLane->PrevBorder = PrevBorder ? PrevBorder->Clone(true) : nullptr;
    NewLane->NextBorder = NextBorder ? NextBorder->Clone(true) : nullptr;
    NewLane->bIsReverse = bIsReverse;
    if (CenterWay) NewLane->CenterWay = CenterWay->Clone(true);
    return NewLane;
}

TRnRef_T<URnLane> URnLane::CreateOneWayLane(TRnRef_T<URnWay> way)
{
    return RnNew<URnLane>(way, nullptr, nullptr, nullptr);
}

TRnRef_T<URnLane> URnLane::CreateEmptyLane(TRnRef_T<URnWay> border, TRnRef_T<URnWay> centerWay)
{
    auto ret = RnNew<URnLane>(nullptr, nullptr, border, border);
    ret->CenterWay = centerWay;
    return ret;
}

void URnLane::AlignBorder(EPLATEAURnLaneBorderType type, EPLATEAURnLaneBorderDir borderDir)
{
    auto border = GetBorder(type);
    if (!border)
        return;
    auto dir = GetBorderDir(type);
    if (dir != borderDir) {
        border->Reverse(true);
    }
}
