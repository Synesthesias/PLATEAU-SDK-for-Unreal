#pragma once

#include <memory>
#include <libplateau_api.h>
#include "plateau/geometry/geo_coordinate.h"

/**
 * 地理院地図のタイル情報です。
 * 具体的なタイル座標情報についてはこちらを参照してください。
 * https://maps.gsi.go.jp/development/tileCoordCheck.html
 */
struct TileCoordinate {
    int column;
    int row;
    int zoom_level;
};

/**
 * 地理院地図のタイル情報とタイル画像の保存先のパスです。
 */
struct VectorTile {
    TileCoordinate coordinate{};
    std::string image_path;
};

class LIBPLATEAU_EXPORT VectorTileDownloader {
public:
    /**
    * \param destination 地理院地図のタイル画像の保存先
    *        extent 経度・緯度・高さの最小・最大で表現される範囲
    *        zoom_level 地理院地図のズームレベル
    */
    VectorTileDownloader(
        const std::string& destination,
        const plateau::geometry::Extent& extent,
        int zoom_level = 15);

    /**
    * \brief 地理院地図のタイル情報から画像をダウンロードし，destinationに保存します。保存されたタイルについての情報はout_vector_tileに格納されます。
    * \param url 地理院地図のurl 種類の詳細はこちらを参照してください　https://maps.gsi.go.jp/development/ichiran.html
    *        destination タイル画像の保存先
    *        coordinate ダウンロードするタイル情報
    *        out_vector_tile ダウンロードされたタイル情報と保存先のパスの格納先
    */
    static void download(const std::string& url, const std::string& destination, const TileCoordinate& coordinate, VectorTile& out_vector_tile);
    /**
    * \brief インデックスに対応したタイル情報から画像をダウンロードします。
    * \param index tiles_のインデックス
    */
    std::shared_ptr<VectorTile> download(int index) const;
    void download(int index, VectorTile& out_vector_tile) const;

    const std::string& getUrl();
    void setUrl(const std::string& value);
    void setExtent(const plateau::geometry::Extent& extent);
    int getTileCount() const;
    TileCoordinate getTile(int index) const;
    static const std::string& getDefaultUrl();

private:
    static inline std::string default_url_ = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";

    std::string url_;
    std::string destination_;
    plateau::geometry::Extent extent_;
    int zoom_level_;
    std::shared_ptr<std::vector<TileCoordinate>> tiles_;

    void updateTileCoordinates();
};
