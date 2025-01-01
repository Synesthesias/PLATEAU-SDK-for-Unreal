#include "Roadnetwork/Structure/RnPoint.h"

RnPoint::RnPoint()
    : Vertex(FVector::ZeroVector) {
}

RnPoint::RnPoint(const FVector& InVertex)
    : Vertex(InVertex) {
}

RnRef_t<RnPoint> RnPoint::Clone() const {
    return RnNew<RnPoint>(Vertex);
}

bool RnPoint::Equals(const RnPoint* X, const RnPoint* Y, float SqrMagnitudeTolerance) {
    if (X == Y) return true;
    if (!X || !Y) return false;
    if (SqrMagnitudeTolerance < 0.0f) return false;

    return Equals(*X, *Y, SqrMagnitudeTolerance);
}

bool RnPoint::Equals(const RnPoint& X, const RnPoint& Y, float SqrMagnitudeTolerance)
{
    // ポインタが同じなら同じ点とみなす
    if (&X == &Y) 
        return true;
    return (X.Vertex - Y.Vertex).SizeSquared() <= SqrMagnitudeTolerance;
}

bool RnPoint::Equals(RnRef_t<const RnPoint> X, RnRef_t<const RnPoint> Y, float SqrMagnitudeTolerance)
{
    return Equals(X.get(), Y.get(), SqrMagnitudeTolerance);
}

bool RnPoint::IsSamePoint(const RnPoint* Other, float SqrMagnitudeTolerance) const {
    return Equals(this, Other, SqrMagnitudeTolerance);
}

bool RnPoint::IsSamePoint(const RnRef_t<RnPoint>& Other, float SqrMagnitudeTolerance) const
{
    return Equals(this, Other.get(), SqrMagnitudeTolerance);
}
