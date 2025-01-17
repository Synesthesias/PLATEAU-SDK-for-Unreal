#include "RoadNetwork/Structure/RnLane.h"

URnLane::URnLane()
    : IsReverse(false) {
}

URnLane::URnLane(const TRnRef_T<URnWay>& InLeftWay, const TRnRef_T<URnWay>& InRightWay,
    const TRnRef_T<URnWay>& InPrevBorder, const TRnRef_T<URnWay>& InNextBorder)
    : IsReverse(false) {
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

bool URnLane::IsValidWay() const {
    return LeftWay && RightWay && LeftWay->IsValid() && RightWay->IsValid();
}

bool URnLane::IsBothConnectedLane() const {
    return PrevBorder && NextBorder;
}

bool URnLane::HasBothBorder() const {
    return PrevBorder && NextBorder;
}

bool URnLane::IsEmptyLane() const {
    return !LeftWay && !RightWay && HasBothBorder();
}

bool URnLane::IsMedianLane() const {
    return Parent ? Parent->IsMedianLane(TRnRef_T<const URnLane>()) : false;
}
ERnLaneBorderDir URnLane::GetBorderDir(ERnLaneBorderType Type) const {
    return IsReverse ?
        (Type == ERnLaneBorderType::Prev ? ERnLaneBorderDir::Right2Left : ERnLaneBorderDir::Left2Right) :
        (Type == ERnLaneBorderType::Prev ? ERnLaneBorderDir::Left2Right : ERnLaneBorderDir::Right2Left);
}

TRnRef_T<URnWay> URnLane::GetBorder(ERnLaneBorderType Type) const {
    return Type == ERnLaneBorderType::Prev ? PrevBorder : NextBorder;
}

void URnLane::SetBorder(ERnLaneBorderType Type, const TRnRef_T<URnWay>& Border) {
    if (Type == ERnLaneBorderType::Prev)
        PrevBorder = Border;
    else
        NextBorder = Border;
}

TRnRef_T<URnWay> URnLane::GetSideWay(ERnDir Dir) const {
    return Dir == ERnDir::Left ? LeftWay : RightWay;
}

void URnLane::SetSideWay(ERnDir Dir, const TRnRef_T<URnWay>& Way) {
    if (Dir == ERnDir::Left)
        LeftWay = Way;
    else
        RightWay = Way;
}

float URnLane::CalcWidth() const {
    if (!IsValidWay()) return 0.0f;

    float TotalWidth = 0.0f;
    int32 Count = 0;

    for (int32 i = 0; i < LeftWay->Count(); ++i) {
        TotalWidth += (RightWay->GetPoint(i)->Vertex - LeftWay->GetPoint(i)->Vertex).Size();
        Count++;
    }

    return Count > 0 ? TotalWidth / Count : 0.0f;
}

void URnLane::Reverse() {
    IsReverse = !IsReverse;
    Swap(PrevBorder, NextBorder);
    if (LeftWay) LeftWay->Reverse(true);
    if (RightWay) RightWay->Reverse(true);
}

void URnLane::BuildCenterWay() {
    if (!IsValidWay()) return;

    auto NewCenterWay = RnNew<URnWay>();
    NewCenterWay->LineString = RnNew<URnLineString>();

    for (int32 i = 0; i < LeftWay->Count(); ++i) {
        auto NewPoint = RnNew<URnPoint>();
        NewPoint->Vertex = (LeftWay->GetPoint(i)->Vertex + RightWay->GetPoint(i)->Vertex) * 0.5f;
        NewCenterWay->LineString->Points->Add(NewPoint);
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

float URnLane::GetCenterLength2D(EAxisPlane Plane) const {
    if (!CenterWay) return 0.0f;

    float Length = 0.0f;
    for (int32 i = 0; i < CenterWay->Count() - 1; ++i) {
        FVector2D Start = FRnDef::To2D(CenterWay->GetPoint(i)->Vertex);
        FVector2D End = FRnDef::To2D(CenterWay->GetPoint(i + 1)->Vertex);
        Length += FVector2D::Distance(Start, End);
    }
    return Length;
}

float URnLane::GetCenterTotalAngle2D() const {
    return CenterWay ? CenterWay->LineString->CalcTotalAngle2D() : 0.0f;
}

float URnLane::GetCenterCurvature2D() const {
    float Length = GetCenterLength2D();
    return Length > 0.0f ? GetCenterTotalAngle2D() / Length : 0.0f;
}

float URnLane::GetCenterRadius2D() const {
    float Curvature = GetCenterCurvature2D();
    return Curvature > 0.0f ? 1.0f / Curvature : MAX_FLT;
}

float URnLane::GetCenterInverseRadius2D() const {
    return GetCenterCurvature2D();
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

TRnRef_T<URnLane> URnLane::Clone() const {
    auto NewLane = RnNew<URnLane>();
    NewLane->LeftWay = LeftWay ? LeftWay->Clone(true) : nullptr;
    NewLane->RightWay = RightWay ? RightWay->Clone(true) : nullptr;
    NewLane->PrevBorder = PrevBorder ? PrevBorder->Clone(true) : nullptr;
    NewLane->NextBorder = NextBorder ? NextBorder->Clone(true) : nullptr;
    NewLane->IsReverse = IsReverse;
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

