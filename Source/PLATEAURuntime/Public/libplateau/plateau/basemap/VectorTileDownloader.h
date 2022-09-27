#pragma once

#include <httplib.h>
#include <libplateau_api.h>
#include <plateau/basemap/VectorTileLoader.h>

class VectorTileDownloader {
public:
    VectorTileDownloader() = default;

    static VectorTile download(std::string uri, TileCoordinate coordinate);
};
