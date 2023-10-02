#pragma once
#include <libplateau_api.h>
#include <plateau/geometry/geo_coordinate.h>
#include <string>

namespace plateau::texture {

    struct LIBPLATEAU_EXPORT MapZoomLevelSearchResult {
        MapZoomLevelSearchResult(bool is_valid, int available_zoom_level_min, int available_zoom_level_max) :
                is_valid_(is_valid), available_zoom_level_min_(available_zoom_level_min),
                available_zoom_level_max_(available_zoom_level_max) {

        }

        bool is_valid_;
        int available_zoom_level_min_;
        int available_zoom_level_max_;
    };

    class LIBPLATEAU_EXPORT MapZoomLevelSearcher {
    public:
        static MapZoomLevelSearchResult search(const std::string& url_template, geometry::GeoCoordinate geo_coord);
    };
}
