#pragma once
#include <libplateau_api.h>
#include "plateau/polygon_mesh/model.h"
#include "plateau/height_map_generator/heightmap_types.h"

namespace plateau::heightMapAligner {

    /// 高さマップと、そこから高さを導くためのmin,max等の情報をまとめてHeightMapFrameと名付けます。
    class LIBPLATEAU_EXPORT HeightMapFrame {
    public:
        HeightMapFrame(plateau::heightMapGenerator::HeightMapT heightmap, int map_width, int map_height, float min_x_arg, float max_x_arg,
                       float min_y_arg, float max_y_arg, float min_height_arg, float max_height_arg) :
                heightmap(std::move(heightmap)), map_width(map_width), map_height(map_height),
                min_x(min_x_arg), max_x(max_x_arg), max_y(max_y_arg), min_y(min_y_arg),
                min_height(min_height_arg), max_height(max_height_arg)
        {
            if(min_x > max_x) std::swap(min_x, max_x);
            if(min_y > max_y) std::swap(min_y, max_y);
            if(min_height > max_height) std::swap(min_height, max_height);
        }
        plateau::heightMapGenerator::HeightMapT heightmap;
        int map_width;
        int map_height;
        float min_x;
        float max_x;
        float min_y;
        float max_y;
        float min_height;
        float max_height;

        /// 場所から、ハイトマップ上の高さを返します。
        /// offset_mapは、ハイトマップの値でどれだけずらすかを指定します。
        double posToHeight(TVec2d pos, double height_offset) const;

        /// 場所から、ハイトマップ上の位置を返します。
        TVec2d posToMapPos(const TVec2d& pos) const;
    };

    /// モデルの高さを、高さマップの高さに合わせます。
    /// 使い方は、合わせたい高さマップを addHeightmapFrame で追加し、最後に Align で合わせます。
    /// 地形が複数ある場合を想定し、合わせたい高さマップが複数ある場合は addHeightmapFrame をその数の回数だけ呼び出してください。
    /// 複数ある場合、位置が合う高さマップが利用されます。
    /// 逆に高さマップをモデルに合わせたい場合は、alignInvert を利用してください。
    class LIBPLATEAU_EXPORT HeightMapAligner {
    public:

        /// コンストラクタで高さのオフセットを指定します。
        explicit HeightMapAligner(double height_offset) : height_offset(height_offset) {}

        void addHeightmapFrame(const HeightMapFrame& heightmap_frame);

        /// モデルの高さを高さマップに合わせます。
        /// max_edge_lengthは、モデルを分割するときの最大の辺の長さで、おおむね4mくらいを想定します。
        void align(plateau::polygonMesh::Model& model, float max_edge_length);

        /// 高さマップをモデルに合わせます。
        /// 変更後の高さマップは getHeightMapFrameAt で取得できます。
        /// alpha_expand_width_cartesianは、アルファマップの平滑化処理において不透明部分を広げる幅（直交座標系）です。
        /// alpha_averaging_width_cartesianは、アルファマップの平滑化処理において平均化する範囲（直交座標系）です。
        /// 上記2つの引数は、要調整ですが 2mくらいが良さそうです(Unityなら2, Unrealなら200)。
        /// height_offsetは、高さマップを対象モデルからの相対でどの高さに合わせるかです。-0.15m くらいが良さそうです。（直交座標系）
        /// skip_threshold_distance は、高さマップとメッシュの高さの差のしきい値で、これを超える箇所は、高さ方向の隙間を尊重して高さ合わせしません。 0.5m くらいが良さそうです。
        void alignInvert(plateau::polygonMesh::Model& model, int alpha_expand_width_cartesian, int alpha_averaging_width_cartesian, double height_offset, const float skip_threshold_of_map_land_distance);
        int heightmapCount() const;
        HeightMapFrame& getHeightMapFrameAt(int index);
    private:
        std::vector<HeightMapFrame> height_map_frames;
        const double height_offset;
    };
}
