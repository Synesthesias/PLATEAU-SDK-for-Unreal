#pragma once
#include <vector>
#include <cstdint>
#include <limits>

namespace plateau::heightMapGenerator {
    /// ハイトマップの1ピクセルの型です。
    using HeightMapElemT = uint16_t;

    /// ピクセルを並べてハイトマップにしたものです。
    using HeightMapT = std::vector<HeightMapElemT>;

    /// ハイトマップの1ピクセルが取りうる最大値です。
    constexpr HeightMapElemT HeightMapNumericMax = std::numeric_limits<HeightMapElemT>::max();

    using AlphaMapElemT = float;
    using AlphaMapT = std::vector<AlphaMapElemT>;
}
