#pragma once
// ☀
#include "CoreMinimal.h"

// グラフ構造用
UENUM(Meta = (Flags))
enum class ERRoadTypeMask : uint8 {
    // 何もなし
    Empty = 0,

    // 車道
    Road = 1 << 0,

    // 歩道
    SideWalk = 1 << 1,

    // 中央分離帯
    Median = 1 << 2,

    // 高速道路
    HighWay = 1 << 3,

    // 不正な値
    Undefined = 1 << 4,

    // 全ての値
    All = ~0
};
ENUM_CLASS_FLAGS(ERRoadTypeMask);

class FRRoadTypeEx {
public:
    // 車道部分
    static bool IsRoad(ERRoadTypeMask Self) {
        return EnumHasAnyFlags(Self, ERRoadTypeMask::Road);
    }

    // 交通道路
    static bool IsHighWay(ERRoadTypeMask Self) {
        return EnumHasAnyFlags(Self, ERRoadTypeMask::HighWay);
    }

    // 歩道
    static bool IsSideWalk(ERRoadTypeMask Self) {
        return EnumHasAnyFlags(Self, ERRoadTypeMask::SideWalk);
    }

    // 中央分離帯
    static bool IsMedian(ERRoadTypeMask Self) {
        return EnumHasAnyFlags(Self, ERRoadTypeMask::Median);
    }

    // selfがflagのどれかを持っているかどうか
    static bool HasAnyFlag(ERRoadTypeMask Self, ERRoadTypeMask Flag) {
        return EnumHasAnyFlags(Self, Flag);
    }
};

