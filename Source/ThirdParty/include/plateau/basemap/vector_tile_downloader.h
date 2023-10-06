#pragma once

#include <memory>
#include <libplateau_api.h>
#include <filesystem>
#include <utility>
#include "plateau/geometry/geo_coordinate.h"

namespace httplib{ class Result; }

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

    TileCoordinate(int column, int row, int zoom_level) :
            column(column), row(row), zoom_level(zoom_level) {};

    TileCoordinate() : TileCoordinate(-1, -1, -1) {};
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
 * VectorTileの集合です。
 * タイル座標の最小最大も提供します。
 * ズームレベルはすべて同じと仮定します。
 */
class VectorTiles {
public:
    VectorTiles(std::vector<VectorTile> tiles);
    std::vector<VectorTile>& tiles() {return tiles_;};
    int minColumn() const{return min_column_;};
    int minRow() const{return min_row_;};
    int maxColumn() const{return max_column_;};
    int maxRow() const{return max_row_;};
    int zoomLevel() const {return zoom_level_;};

    /// 緯度経度の最小と最大をExtent型で返します。
    plateau::geometry::Extent extent() const;

    /**
     * タイルのダウンロードに1つでも成功していればtrueを返します。
     */
    bool anyTileSucceed();
    const VectorTile& firstSucceed() const;

    /**
     * タイル集合のうち、タイル座標が引数に相当するものを探して返します。
     */
    const VectorTile& getTile(int column, int row) const;
private:
    std::vector<VectorTile> tiles_;
    int min_column_;
    int min_row_;
    int max_column_;
    int max_row_;
    int zoom_level_;
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
    * \param url_template 地図のURLであって、タイル座標のプレースホルダとして文字列"{z}","{y}","{x}"を含むものです。 種類の詳細はこちらを参照してください　https://maps.gsi.go.jp/development/ichiran.html
    * \param destination タイル画像の保存先
    * \param coordinate ダウンロードするタイル情報
    * \param out_vector_tile ダウンロードされたタイル情報と保存先のパスの格納先
    */
    static void download(const std::string& url_template, const std::string& destination, const TileCoordinate& coordinate, VectorTile& out_vector_tile);

    /**
    * \brief 地理院地図のタイル情報から画像をダウンロードし，destinationに保存します。保存されたタイルについての情報はout_vector_tileに格納されます。
    * \param url_template 地図のURLであって、タイル座標のプレースホルダとして文字列"{z}","{y}","{x}"を含むものです。 種類の詳細はこちらを参照してください　https://maps.gsi.go.jp/development/ichiran.html
    * \param destination タイル画像の保存先
    * \param coordinate ダウンロードするタイル情報
    * \return ダウンロードされたタイル情報と保存先のパスの格納先
    */
    static std::shared_ptr<VectorTile> download(const std::string& url_template, const std::string& destination, const TileCoordinate& coordinate);

    /**
    * \brief インデックスに対応したタイル情報から画像をダウンロードします。
    * \param index tiles_のインデックス
    */
    std::shared_ptr<VectorTile> download(int index) const;
    bool download(int index, VectorTile& out_vector_tile) const;
    static httplib::Result httpRequest(const std::string& url_template, TileCoordinate tile_coordinate, std::string& out_body);

    /**
    * メンバー変数である範囲、ズームレベルに該当する地図タイルをすべてダウンロードし、そのタイル情報を返します。
    */
    VectorTiles downloadAll() const;

    /**
     * TileCoordinateの地図タイルをダウンロードしたとき、その画像ファイルがどこに配置されるべきかを返します。
     * 具体的には、与えられたパス(destination)の末尾にzoom_levelフォルダ、columnフォルダ、row.extension を付与したパスを返します。
     */
    static std::filesystem::path calcDestinationPath(const TileCoordinate& coord, const std::string& destination, const std::string& file_extension);
    std::filesystem::path calcDestinationPath(int index) const;

    const std::string& getUrl() const;
    /// ここでいうURLとは、タイル画像の座標をプレースホルダとして文字列 "{x}","{y}","{z}"を含めたものを想定します。
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
