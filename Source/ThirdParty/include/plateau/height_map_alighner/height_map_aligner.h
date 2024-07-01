#pragma once
#include <libplateau_api.h>
#include "plateau/polygon_mesh/model.h"

namespace plateau::heightMapAligner {

    /// 高さマップと、そこから高さを導くためのmin,max等の情報をまとめてHeightMapFrameと名付けます。
    class LIBPLATEAU_EXPORT HeightMapFrame {
        using MapValueT = uint16_t;
    public:
        HeightMapFrame(std::vector<uint16_t> heightmap, int map_width, int map_height, float min_x_arg, float max_x_arg,
                       float min_y_arg, float max_y_arg, float min_height_arg, float max_height_arg) :
                heightmap(std::move(heightmap)), map_width(map_width), map_height(map_height), min_x(min_x_arg), max_x(max_x_arg), max_y(max_y_arg), min_y(min_y_arg), min_height(min_height_arg), max_height(max_height_arg)
        {
            if(min_x > max_x) std::swap(min_x, max_x);
            if(min_y > max_y) std::swap(min_y, max_y);
            if(min_height > max_height) std::swap(min_height, max_height);
        }
        std::vector<MapValueT> heightmap;
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
        void align(plateau::polygonMesh::Model& model);

        /// 高さマップをモデルに合わせます。
        /// 変更後の高さマップは getHeightMapFrameAt で取得できます。
        void alignInvert(plateau::polygonMesh::Model& model);
        int heightmapCount() const;
        HeightMapFrame& getHeightMapFrameAt(int index);
    private:
        std::vector<HeightMapFrame> height_map_frames;
        const double height_offset;
    };
}
