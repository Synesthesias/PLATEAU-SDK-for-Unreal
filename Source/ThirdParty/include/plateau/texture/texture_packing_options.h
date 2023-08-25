#pragma once

#include <plateau/geometry/geo_coordinate.h>
#include <citygml/vecs.hpp>
#include <plateau/polygon_mesh/polygon_mesh_utils.h>

namespace plateau::polygonMesh {
    /**
    * テクスチャパッキングのオプションの定義
    */

    struct TexturePackingOptions {
        /// 設定をデフォルト値にするコンストラクタです。
        TexturePackingOptions() : resolution(0) {}

    public:
        int resolution; // テクスチャを結合するテクスチャ画像のサイズ（width:resolution x height:resolution）
    };
}
