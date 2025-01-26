#pragma once

#include "CoreMinimal.h"
#include "RoadNetwork/PLATEAURnDef.h"
#include <optional>

#include "RoadNetwork/GeoGraph/LineSegment3D.h"
#include "RoadNetwork/RGraph/RGraphDef.h"

class UPLATEAUCityObjectGroup;
class URnIntersection;
class URnLineString;
class URnWay;
class URnPoint;
class URnLane;

struct FPLATEAURnLinq
{
private:
    template<typename TSrc, typename TDst>
    static auto SelectImpl(const TArray<TSrc>& Src, TFunction<TDst(const TSrc&)> Selector)
    {
        TArray<TDst> Result;
        Result.Reserve(Src.Num());
        for (const auto& S : Src) {
            Result.Add(Selector(S));
        }
        return Result;
    }
    template<typename TSrc, typename TDst>
    static auto SelectWithIndexImpl(const TArray<TSrc>& Src, TFunction<TDst(const TSrc&, int32 Index)> Selector) {
        TArray<TDst> Result;
        Result.Reserve(Src.Num());
        auto Index = 0;
        for (auto&& E : Src) {
            Result.Add(Selector(E, Index));
            Index++;
        }
        return Result;
    }
public:
    // Linq.Select代わり
    // Src      : 範囲forで回せるコンテナ
    // Selector : Srcの要素を受け取り、変換した値を返す関数
    template<typename T, typename F>
    static auto Select(const TArray<T>& Src, F&& Selector)
    {
        using DstType = decltype(Selector(*Src.begin()));
        return SelectImpl<T, DstType>(Src, Forward<F>(Selector));
    }

    // Linq.Select代わり(index渡すバージョン)
    // Src      : 範囲forで回せるコンテナ
    // Selector : Srcの要素とインデックスを受け取り、変換した値を返す関数
    template<typename T, typename F>
    static auto SelectWithIndex(const TArray<T>& Src, F&& Selector) {

        using DstType = decltype(Selector(*Src.begin()));
        return SelectWithIndexImpl<T, DstType>(Src, Forward<F>(Selector));
    }

private:
    template<typename T>
    static bool TryFirstOrDefaultImpl(const TSet<T>& Set, TFunction<bool(const T&)> Predicate, T& OutV) {
        for (auto& S : Set) {
            if (Predicate(S)) {
                OutV = S;
                return true;
            }
        }
        return false;
    }
public:
    template<typename T, typename F>
    static bool TryFirstOrDefault(const TSet<T>& Set, F&& Predicate, T& OutV);
};

template <typename T, typename F>
bool FPLATEAURnLinq::TryFirstOrDefault(const TSet<T>& Set, F&& Predicate, T& OutV)
{
    return TryFirstOrDefaultImpl<T>(Set, Predicate, OutV);
}
