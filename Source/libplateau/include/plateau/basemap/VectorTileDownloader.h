#pragma once

#include <httplib.h>
#include <memory>
#include <vector>
#include <libplateau_api.h>
#include "plateau/geometry/geo_coordinate.h"


struct TileCoordinate {
    int column;

    int row;

    int zoom_level;
};

struct VectorTile {
    TileCoordinate coordinate;

    std::string image_path;
};

class LIBPLATEAU_EXPORT VectorTileDownloader {
public:
    VectorTileDownloader(std::string uri);

    static std::shared_ptr <VectorTile> download(std::string destination, TileCoordinate coordinate);
    static const char* getUri();
    static void setUri(char* uri);

private:
    static inline std::string uri_ = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";

};
