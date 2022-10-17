#pragma once

#include <vector>

#include <plateau/basemap/VectorTileLoader.h>
#include <plateau/geometry/geo_coordinate.h>

class TileProjection{
public:
    static std::shared_ptr <std::vector<TileCoordinate>> getTileCoordinates(const plateau::geometry::Extent& extent);
    static TileCoordinate project(const plateau::geometry::GeoCoordinate& coordinate);

private:
    static const int zoomLevel = 15;

    static int long2tileX(double longitude, int z);
    static int lat2tileY(double latitude, int z);
};