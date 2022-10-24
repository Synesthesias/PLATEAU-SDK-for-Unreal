#pragma once

#include <plateau/geometry/geo_coordinate.h>

using namespace plateau::geometry;

/**
 * @enum MeshGranularity
 *
 * メッシュの結合単位
 */
enum class MeshGranularity {
    //! 最小地物単位(LOD2, LOD3の各部品)
    PerAtomicFeatureObject,
    //! 主要地物単位(建築物、道路等)
    PerPrimaryFeatureObject,
    //! 都市モデル地域単位(GMLファイル内のすべてを結合)
    PerCityModelArea
};

/**
 * @enum MeshFileFormat
 *
 * 出力ファイルフォーマット
 */
enum class MeshFileFormat {
    OBJ,
    GLTF
};

/**
 * \brief 都市モデルからメッシュへの変換設定です。
 */
struct MeshConvertOptions {
    /**
     * \brief 出力後のメッシュの座標系を指定します。
     */
    CoordinateSystem mesh_axes;

    /**
     * \brief 変換時の基準点を指定します。
     *
     * 基準点は平面直角座標であり、メッシュは基準点からの相対座標で出力されます。
     */
    TVec3d reference_point;

    /**
     * \brief 出力後のメッシュの粒度(結合単位)を指定します。
     */
    MeshGranularity mesh_granularity;

    /**
     * \brief 出力後のメッシュに含める最小のLODを指定します。
     */
    unsigned min_lod;

    /**
     * \brief 出力後のメッシュに含める最大のLODを指定します。
     */
    unsigned max_lod;

    /**
     * \brief 1つの地物について複数のLODがある場合に最大LOD以外のジオメトリを出力するかどうかを指定します。
     */
    bool export_lower_lod;

    /**
     * \brief テクスチャ、マテリアル情報を出力するかどうかを指定します。
     */
    bool export_appearance;

    /**
     * \brief メートル法基準での単位の倍率を指定します。
     *
     * Unreal Engine向けには0.01に設定し、cm単位で出力してください。
     */
    float unit_scale;

    /**
     * \brief 出力ファイルフォーマットを指定します。
     */
    MeshFileFormat mesh_file_format;

    /**
     * 次のWebサイトで示される座標系番号です。
     * https://www.gsi.go.jp/sokuchikijun/jpc.html
     */
    int coordinate_zone_id;

    MeshConvertOptions() :
            mesh_axes(CoordinateSystem::WUN),
            mesh_file_format(MeshFileFormat::OBJ),
            reference_point(),
            mesh_granularity(MeshGranularity::PerPrimaryFeatureObject),
            min_lod(0),
            max_lod(3),
            export_lower_lod(true),
            export_appearance(true),
            unit_scale(1),
            coordinate_zone_id(9){
    }
};
