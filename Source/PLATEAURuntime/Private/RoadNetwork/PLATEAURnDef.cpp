#include "RoadNetwork/PLATEAURnDef.h"


EPLATEAURnLaneBorderType FPLATEAURnLaneBorderTypeEx::GetOpposite(EPLATEAURnLaneBorderType dir)
{
    return dir == EPLATEAURnLaneBorderType::Prev ? EPLATEAURnLaneBorderType::Next : EPLATEAURnLaneBorderType::Prev;
}

EPLATEAURnDir FPLATEAURnDirEx::GetOpposite(EPLATEAURnDir dir)
{
    return dir == EPLATEAURnDir::Left ? EPLATEAURnDir::Right : EPLATEAURnDir::Left;
}

EPLATEAURnLaneBorderDir FPLATEAURnLaneBorderDirEx::GetOpposite(EPLATEAURnLaneBorderDir dir)
{
    return dir == EPLATEAURnLaneBorderDir::Left2Right ? EPLATEAURnLaneBorderDir::Right2Left : EPLATEAURnLaneBorderDir::Left2Right;
}

UObject* FPLATEAURnDef::GetNewObjectWorld()
{
    if (NewObjectWorld)
        return NewObjectWorld;
    return GetTransientPackage();
}

void FPLATEAURnDef::SetNewObjectWorld(UObject* World)
{
    NewObjectWorld = World;
}

inline FVector2D FPLATEAURnDef::To2D(const FVector& Vector) {
    return FAxisPlaneEx::ToVector2D(Vector, Plane);
}
