#include "RoadNetwork/Structure/RnLane.h"

RnLane::RnLane()
    : IsReverse(false) {
}

RnLane::RnLane(const RnRef_t<RnWay>& InLeftWay, const RnRef_t<RnWay>& InRightWay,
    const RnRef_t<RnWay>& InPrevBorder, const RnRef_t<RnWay>& InNextBorder)
    : PrevBorder(InPrevBorder)
    , NextBorder(InNextBorder)
    , LeftWay(InLeftWay)
    , RightWay(InRightWay)
    , IsReverse(false) {
}

TArray<RnRef_t<RnWay>> RnLane::GetBothWays() const {
    TArray<RnRef_t<RnWay>> Ways;
    if (LeftWay) Ways.Add(LeftWay);
    if (RightWay) Ways.Add(RightWay);
    return Ways;
}

TArray<RnRef_t<RnWay>> RnLane::GetAllBorders() const {
    TArray<RnRef_t<RnWay>> Borders;
    if (PrevBorder) Borders.Add(PrevBorder);
    if (NextBorder) Borders.Add(NextBorder);
    return Borders;
}

TArray<RnRef_t<RnWay>> RnLane::GetAllWays() const {
    TArray<RnRef_t<RnWay>> Ways = GetBothWays();
    Ways.Append(GetAllBorders());
    return Ways;
}

bool RnLane::IsValidWay() const {
    return LeftWay && RightWay && LeftWay->IsValid() && RightWay->IsValid();
}

bool RnLane::IsBothConnectedLane() const {
    return PrevBorder && NextBorder;
}

bool RnLane::HasBothBorder() const {
    return PrevBorder && NextBorder;
}

bool RnLane::IsEmptyLane() const {
    return !LeftWay && !RightWay && HasBothBorder();
}

bool RnLane::IsMedianLane() const {
    return Parent ? Parent->IsMedianLane(RnRef_t<const RnLane>()) : false;
}
ERnLaneBorderDir RnLane::GetBorderDir(ERnLaneBorderType Type) const {
    return IsReverse ?
        (Type == ERnLaneBorderType::Prev ? ERnLaneBorderDir::Right2Left : ERnLaneBorderDir::Left2Right) :
        (Type == ERnLaneBorderType::Prev ? ERnLaneBorderDir::Left2Right : ERnLaneBorderDir::Right2Left);
}

RnRef_t<RnWay> RnLane::GetBorder(ERnLaneBorderType Type) const {
    return Type == ERnLaneBorderType::Prev ? PrevBorder : NextBorder;
}

void RnLane::SetBorder(ERnLaneBorderType Type, const RnRef_t<RnWay>& Border) {
    if (Type == ERnLaneBorderType::Prev)
        PrevBorder = Border;
    else
        NextBorder = Border;
}

RnRef_t<RnWay> RnLane::GetSideWay(ERnDir Dir) const {
    return Dir == ERnDir::Left ? LeftWay : RightWay;
}

void RnLane::SetSideWay(ERnDir Dir, const RnRef_t<RnWay>& Way) {
    if (Dir == ERnDir::Left)
        LeftWay = Way;
    else
        RightWay = Way;
}

float RnLane::CalcWidth() const {
    if (!IsValidWay()) return 0.0f;

    float TotalWidth = 0.0f;
    int32 Count = 0;

    for (int32 i = 0; i < LeftWay->Count(); ++i) {
        TotalWidth += (RightWay->GetPoint(i)->Vertex - LeftWay->GetPoint(i)->Vertex).Size();
        Count++;
    }

    return Count > 0 ? TotalWidth / Count : 0.0f;
}

void RnLane::Reverse() {
    IsReverse = !IsReverse;
    Swap(PrevBorder, NextBorder);
    if (LeftWay) LeftWay->Reverse(true);
    if (RightWay) RightWay->Reverse(true);
}

void RnLane::BuildCenterWay() {
    if (!IsValidWay()) return;

    auto NewCenterWay = RnNew<RnWay>();
    NewCenterWay->LineString = RnNew<RnLineString>();

    for (int32 i = 0; i < LeftWay->Count(); ++i) {
        auto NewPoint = RnNew<RnPoint>();
        NewPoint->Vertex = (LeftWay->GetPoint(i)->Vertex + RightWay->GetPoint(i)->Vertex) * 0.5f;
        NewCenterWay->LineString->Points->Add(NewPoint);
    }

    CenterWay = NewCenterWay;
}

RnRef_t<RnWay> RnLane::GetCenterWay() {
    if (!CenterWay) BuildCenterWay();
    return CenterWay;
}

void RnLane::GetNearestCenterPoint(const FVector& Pos, FVector& OutNearest, float& OutPointIndex, float& OutDistance) const {
    if (!CenterWay) {
        const_cast<RnLane*>(this)->BuildCenterWay();
    }
    CenterWay->GetNearestPoint(Pos, OutNearest, OutPointIndex, OutDistance);
}

float RnLane::GetCenterLength() const {
    return CenterWay ? CenterWay->CalcLength() : 0.0f;
}

float RnLane::GetCenterLength2D(EAxisPlane Plane) const {
    if (!CenterWay) return 0.0f;

    float Length = 0.0f;
    for (int32 i = 0; i < CenterWay->Count() - 1; ++i) {
        FVector2D Start = FRnDef::To2D(CenterWay->GetPoint(i)->Vertex);
        FVector2D End = FRnDef::To2D(CenterWay->GetPoint(i + 1)->Vertex);
        Length += FVector2D::Distance(Start, End);
    }
    return Length;
}

float RnLane::GetCenterTotalAngle2D() const {
    return CenterWay ? CenterWay->LineString->CalcTotalAngle2D() : 0.0f;
}

float RnLane::GetCenterCurvature2D() const {
    float Length = GetCenterLength2D();
    return Length > 0.0f ? GetCenterTotalAngle2D() / Length : 0.0f;
}

float RnLane::GetCenterRadius2D() const {
    float Curvature = GetCenterCurvature2D();
    return Curvature > 0.0f ? 1.0f / Curvature : MAX_FLT;
}

float RnLane::GetCenterInverseRadius2D() const {
    return GetCenterCurvature2D();
}

float RnLane::GetDistanceFrom(const FVector& Point) const {
    if (!IsValidWay()) return MAX_FLT;

    FVector Nearest;
    float PointIndex, Distance;
    GetNearestCenterPoint(Point, Nearest, PointIndex, Distance);
    return Distance;
}

bool RnLane::IsInside(const FVector& Point) const {
    if (!IsValidWay()) return false;

    FVector Nearest;
    float Distance;
    return !LeftWay->IsOutSide(Point, Nearest, Distance) &&
        !RightWay->IsOutSide(Point, Nearest, Distance);
}

RnRef_t<RnLane> RnLane::Clone() const {
    auto NewLane = RnNew<RnLane>();
    NewLane->LeftWay = LeftWay ? LeftWay->Clone(true) : nullptr;
    NewLane->RightWay = RightWay ? RightWay->Clone(true) : nullptr;
    NewLane->PrevBorder = PrevBorder ? PrevBorder->Clone(true) : nullptr;
    NewLane->NextBorder = NextBorder ? NextBorder->Clone(true) : nullptr;
    NewLane->IsReverse = IsReverse;
    if (CenterWay) NewLane->CenterWay = CenterWay->Clone(true);
    return NewLane;
}

RnRef_t<RnLane> RnLane::CreateOneWayLane(RnRef_t<RnWay> way)
{
    return RnNew<RnLane>(way, nullptr, nullptr, nullptr);
}

RnRef_t<RnLane> RnLane::CreateEmptyLane(RnRef_t<RnWay> border, RnRef_t<RnWay> centerWay)
{
    auto ret = RnNew<RnLane>(nullptr, nullptr, border, border);
    ret->CenterWay = centerWay;
    return ret;
}

