#pragma once

#include "CoreMinimal.h"
#include "../RnDef.h"
#include <optional>

#include "RoadNetwork/GeoGraph/LineSegment3D.h"

class UPLATEAUCityObjectGroup;
class RnIntersection;
class RnLineString;
class RnWay;
class RnPoint;
class RnLane;


class FLineCrossPointResult {
public:
    class FTargetLineInfo {
    public:
        RnRef_t<RnLineString> LineString;
        TArray<TTuple<float, FVector>> Intersections;
    };

    TArray<RnRef_t<FTargetLineInfo>> TargetLines;
    FLineSegment3D LineSegment;
};

struct RnEx
{
public:
    static TArray<RnRef_t<UPLATEAUCityObjectGroup>> GetSceneSelectedCityObjectGroups();

    template<typename T>
    static void Replace(TArray<T>& Self, T Before, T After);

    static void ReplaceLane(TArray<RnRef_t<RnLane>>& Self, RnRef_t<RnLane> Before, RnRef_t<RnLane> After);

    static RnRef_t<RnLineString> CreateInnerLerpLineString(
        const TArray<FVector>& LeftVertices,
        const TArray<FVector>& RightVertices,
        RnRef_t<RnPoint> Start,
        RnRef_t<RnPoint> End,
        RnRef_t<RnWay> StartBorder,
        RnRef_t<RnWay> EndBorder,
        float T,
        float PointSkipDistance = 1e-3f);

    static RnRef_t<FLineCrossPointResult> GetLineIntersections(
        const FLineSegment3D& LineSegment,
        const TArray<RnRef_t<RnWay>>& Ways);


    template<typename T, typename U>
    static TArray<U> Map(const TArray<T>& Src, TFunction< U(const T&)> Selector)
    {
        TArray<U> Result;
        Result.Reserve(Src.Num());
        for (const auto& S : Src) {
            Result.Add(Selector(S));
        }
        return Result;
    }

    template<typename T>
    static int32 Compare(T A, T B)
    {
        if (A < B)
            return -1;
        if (A > B)
            return 1;
        return 0;
    }
};
template<typename T>
void RnEx::Replace(TArray<T>& Self, T Before, T After) {
    for (int32 i = 0; i < Self.Num(); i++) {
        if (Self[i] == Before) {
            Self[i] = After;
        }
    }
}

