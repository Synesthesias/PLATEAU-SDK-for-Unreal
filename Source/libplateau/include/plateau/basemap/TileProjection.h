#pragma once

#include <vector>

#include <plateau/basemap/VectorTileDownloader.h>
#include <plateau/geometry/geo_coordinate.h>

class LIBPLATEAU_EXPORT TileProjection {
public:
    static std::shared_ptr <std::vector<TileCoordinate>> getTileCoordinates(const plateau::geometry::Extent& extent, int zoomLevel);
    static std::shared_ptr <TileCoordinate> project(const plateau::geometry::GeoCoordinate& coordinate, int zoomLevel);
    static std::shared_ptr <plateau::geometry::GeoCoordinate> unproject(TileCoordinate& coordinate, int zoomLevel);
};
