#pragma once
#include <libplateau_api.h>
#include <plateau/geometry/geo_coordinate.h>
#include <string>

namespace plateau::texture {

    /**
     * 指定の地図タイルURLが利用可能かどうか、利用可能な最小と最大のズームレベルが何かを調べた結果です。
     */
    struct LIBPLATEAU_EXPORT MapZoomLevelSearchResult {
        MapZoomLevelSearchResult(bool is_valid, int available_zoom_level_min, int available_zoom_level_max) :
                is_valid_(is_valid), available_zoom_level_min_(available_zoom_level_min),
                available_zoom_level_max_(available_zoom_level_max) {

        }

        bool is_valid_;
        int available_zoom_level_min_;
        int available_zoom_level_max_;
    };

    /**
     * 指定の地図タイルURLが利用可能かどうか、利用可能な最小と最大のズームレベルが何かを調べます。
     */
    class LIBPLATEAU_EXPORT MapZoomLevelSearcher {
    public:
        static MapZoomLevelSearchResult search(const std::string& url_template, geometry::GeoCoordinate geo_coord);
    private:
        static constexpr int zoom_level_search_range_min = 1;
        static constexpr int zoom_level_search_range_max = 20;
    };
}
