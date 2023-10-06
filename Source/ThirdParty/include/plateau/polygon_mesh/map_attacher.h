#pragma once

#include <plateau/polygon_mesh/model.h>
#include <plateau/geometry/geo_reference.h>
#include <filesystem>

namespace  plateau::polygonMesh {
    using namespace plateau::geometry;
    /**
     * モデルに航空写真または地図を貼り付けます。
     */
    class LIBPLATEAU_EXPORT MapAttacher {
    public:
        /**
         * 引数のModelに含まれる各Meshに対し、航空写真または地図を貼り付けます。
         * 正常に読み込むことができた地図タイルの数を返します。
         */
        int attach(Model& model, const std::string& map_url_template, const std::filesystem::path& map_download_dest, const int zoom_level, const GeoReference& geo_reference);
    };

}
