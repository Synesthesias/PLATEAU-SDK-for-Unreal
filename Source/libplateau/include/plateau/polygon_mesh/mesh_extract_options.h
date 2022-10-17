#pragma once

#include <plateau/io/mesh_convert_options.h>
#include <citygml/vecs.hpp>

namespace plateau::polygonMesh {

    struct MeshExtractOptions {
        MeshExtractOptions(TVec3d reference_point, CoordinateSystem mesh_axes, MeshGranularity mesh_granularity,
                           unsigned max_lod, unsigned min_lod, bool export_appearance, int grid_count_of_side,
                           float unit_scale, int coordinate_zone_id, Extent extent) :
                reference_point(reference_point),
                mesh_axes(mesh_axes),
                mesh_granularity(mesh_granularity),
                max_lod(max_lod),
                min_lod(min_lod),
                export_appearance(export_appearance),
                grid_count_of_side(grid_count_of_side),
                unit_scale(unit_scale),
                coordinate_zone_id(coordinate_zone_id),
                extent(extent){}

    public:
        TVec3d reference_point;
        CoordinateSystem mesh_axes;
        MeshGranularity mesh_granularity;
        unsigned max_lod;
        unsigned min_lod;
        bool export_appearance;
        /**
         * グリッド分けする時の、1辺の分割数です。
         * この数の2乗がグリッドの数となり、実際にはそれより細かくグループ分けされます。
         */
        int grid_count_of_side;
        float unit_scale;

        /**
         * 国土交通省が規定する、日本における平面直角座標系の基準点のうちどれを採用するかを番号で指定します。
         * 基準点と番号は次のWebサイトに記載のとおりです。
         * https://www.gsi.go.jp/sokuchikijun/jpc.html
         */
        int coordinate_zone_id;
        Extent extent;
    };
}
