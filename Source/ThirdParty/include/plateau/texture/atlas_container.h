#pragma once

#include <cstddef>

namespace plateau::texture {
    class AtlasContainer {
    public:

        explicit AtlasContainer(size_t _gap, size_t _horizontal_range, size_t _vertical_range);

        size_t getGap() const {
            return gap;
        }
        size_t getRootHorizontalRange() const {
            return root_horizontal_range;
        }
        size_t getHorizontalRange() const {
            return horizontal_range;
        }
        size_t getVerticalRange() const {
            return vertical_range;
        }
        void add(const size_t _length);

    private:
        size_t gap;                     // 画像を格納するコンテナの高さ
        size_t root_horizontal_range;   // コンテナ内で未使用の領域のX座標
        size_t horizontal_range;        // 最後の画像をパッキングするための領域のX座標
        size_t vertical_range;          // パッキングの対象となる親の画像のコンテナが配置されている左上のY座標
    };
}
