#pragma once

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

    /**
     * \brief 出力後のメッシュに含める最大のLODを指定します。
     */
    std::string image;
};

class LIBPLATEAU_EXPORT VectorTileLoader{
public:
    VectorTileLoader() = default;

    static std::shared_ptr<std::vector<VectorTile>> load(plateau::geometry::Extent extent);
    static char* getUri();

private:
    static inline std::string uri_ = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";
};