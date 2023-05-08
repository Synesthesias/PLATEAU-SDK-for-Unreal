#pragma once

#include <memory>
#include <libplateau_api.h>
#include <filesystem>
#include "plateau/geometry/geo_coordinate.h"

/**
 * Http通信のエラーです。(Httplib::Error)
 */
enum class HttpResult {
    Success = 0,
    Unknown,
    Connection,
    BindIPAddress,
    Read,
    Write,
    ExceedRedirectCount,
    Canceled,
    SSLConnection,
    SSLLoadingCerts,
    SSLServerVerification,
    UnsupportedMultipartBoundaryChars,
    Compression,
    ConnectionTimeout,
};

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
    HttpResult result;
};

/**
 * 地理院地図タイルをダウンロードして画像ファイルとして保存します。
 */
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
    * \param destination タイル画像の保存先
    * \param coordinate ダウンロードするタイル情報
    * \param out_vector_tile ダウンロードされたタイル情報と保存先のパスの格納先
    */
    static void download(const std::string& url, const std::string& destination, const TileCoordinate& coordinate, VectorTile& out_vector_tile);

    /**
    * \brief 地理院地図のタイル情報から画像をダウンロードし，destinationに保存します。保存されたタイルについての情報はout_vector_tileに格納されます。
    * \param url 地理院地図のurl 種類の詳細はこちらを参照してください　https://maps.gsi.go.jp/development/ichiran.html
    * \param destination タイル画像の保存先
    * \param coordinate ダウンロードするタイル情報
    * \return ダウンロードされたタイル情報と保存先のパスの格納先
    */
    static std::shared_ptr<VectorTile> download(const std::string& url, const std::string& destination, const TileCoordinate& coordinate);

    /**
    * \brief インデックスに対応したタイル情報から画像をダウンロードします。
    * \param index tiles_のインデックス
    */
    std::shared_ptr<VectorTile> download(int index) const;
    bool download(int index, VectorTile& out_vector_tile) const;

    /// TileCoordinateの地図タイルをダウンロードしたとき、その画像ファイルがどこに配置されるべきかを返します。
    static std::filesystem::path calcDestinationPath(const TileCoordinate& coord, const std::string& destination);
    std::filesystem::path calcDestinationPath(int index) const;

    const std::string& getUrl();
    void setUrl(const std::string& value);
    void setExtent(const plateau::geometry::Extent& extent);
    int getTileCount() const;
    TileCoordinate getTile(int index);
    static const std::string& getDefaultUrl();

private:
    static const std::string default_url_;

    std::string url_;
    std::string destination_;
    plateau::geometry::Extent extent_;
    int zoom_level_;
    std::shared_ptr<std::vector<TileCoordinate>> tiles_;

    void updateTileCoordinates();
};
