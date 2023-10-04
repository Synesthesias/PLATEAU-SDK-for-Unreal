#pragma once
#include <libplateau_api.h>
#include <plateau/polygon_mesh/model.h>

namespace plateau::granularityConvert {


    class LIBPLATEAU_EXPORT GranularityConvertOption {
    public:
        GranularityConvertOption(polygonMesh::MeshGranularity granularity, int grid_count)
                : granularity_(granularity), grid_count_(grid_count) {};
        polygonMesh::MeshGranularity granularity_;
        int grid_count_; // 粒度が地域単位のときのみに利用します。
    };

    class LIBPLATEAU_EXPORT GranularityConverter {
    public:
         plateau::polygonMesh::Model convert(plateau::polygonMesh::Model& src, GranularityConvertOption option);
    };
}
