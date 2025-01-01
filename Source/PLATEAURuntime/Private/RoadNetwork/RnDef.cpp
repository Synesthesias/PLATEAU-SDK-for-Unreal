#include "RoadNetwork/RnDef.h"

#include "RoadNetwork/Util/VectorEx.h"

ERnLaneBorderType FRnLaneBorderTypeEx::GetOpposite(ERnLaneBorderType dir)
{
    return dir == ERnLaneBorderType::Prev ? ERnLaneBorderType::Next : ERnLaneBorderType::Prev;
}

ERnDir FRnDirEx::GetOpposite(ERnDir dir)
{
    return dir == ERnDir::Left ? ERnDir::Right : ERnDir::Left;
}

inline FVector2D FRnDef::To2D(const FVector& Vector) {
    return FAxisPlaneEx::ToVector2D(Vector, Plane);
}
