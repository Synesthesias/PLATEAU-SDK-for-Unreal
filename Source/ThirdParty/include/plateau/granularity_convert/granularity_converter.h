#pragma once
#include <libplateau_api.h>
#include <plateau/polygon_mesh/model.h>

namespace plateau::granularityConvert {

    /**
    * 分割結合用のメッシュの結合単位
    */
    enum class ConvertGranularity {
        //! 最小地物単位(LOD2, LOD3の各部品)
        PerAtomicFeatureObject = 0,
        //! 主要地物単位(建築物、道路等)
        PerPrimaryFeatureObject = 1,
        //! 都市モデル地域単位(GMLファイル内のすべてを結合)
        PerCityModelArea = 2,
        /// 主要地物単位内のマテリアル単位
        MaterialInPrimary = 3
    };

    class LIBPLATEAU_EXPORT GranularityConvertOption {
    public:
        GranularityConvertOption(ConvertGranularity granularity, int grid_count)
                : granularity_(granularity), grid_count_(grid_count) {};
        ConvertGranularity granularity_;
        int grid_count_; // 粒度が地域単位のときのみに利用します。
    };


    class LIBPLATEAU_EXPORT GranularityConverter {
    public:
         /// Modelの粒度を変換したものを作って返します。
         plateau::polygonMesh::Model convert(const plateau::polygonMesh::Model& src, GranularityConvertOption option) const;
    };
}
