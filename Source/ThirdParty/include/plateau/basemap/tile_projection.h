#pragma once

#include <vector>

#include <plateau/basemap/vector_tile_downloader.h>
#include <plateau/geometry/geo_coordinate.h>

class LIBPLATEAU_EXPORT TileProjection {
public:
    /**
    * \brief 経度・緯度・高さの最小・最大で表現される範囲とズームレベルからそれに該当する地理院地図のタイル座標の集合を返します。
    * \param extent 経度・緯度・高さの最小・最大で表現される範囲
    *        zoom_level 地理院地図のズームレベル
    * \return extentの範囲に該当する地理院地図のタイル座標の集合
    */
    static std::shared_ptr<std::vector<TileCoordinate>> getTileCoordinates(const plateau::geometry::Extent& extent, int zoomLevel);

    /**
    * \brief 経度・緯度・高さで表現される座標とズームレベルからそれに該当する地理院地図のタイル座標を返します。
    * \param coordinate 経度・緯度・高さで表現される範囲
    *        zoom_level 地理院地図のズームレベル
    * \return coordinateの座標に該当する地理院地図のタイル座標
    */
    static TileCoordinate project(const plateau::geometry::GeoCoordinate& coordinate, int zoom_level);

    /**
    * \brief 地理院地図のタイル座標とズームレベルからそれに該当する経度・緯度・高さの最小・最大で表現される範囲を返します。
    * \param coordinate 地理院地図のタイル座標
    *        zoom_level 地理院地図のズームレベル
    * \return 入力の地理院地図のタイルの経度・緯度・高さの範囲
    */
    static plateau::geometry::Extent unproject(const TileCoordinate& coordinate);
};
