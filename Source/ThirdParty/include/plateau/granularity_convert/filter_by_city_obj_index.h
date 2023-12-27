#pragma once
#include <plateau/polygon_mesh/mesh.h>
#include <libplateau_api.h>

namespace plateau::granularityConvert {
    class FilterByCityObjIndex {
    public:
        /// MeshのうちCityObjectIndexが引数idに一致する箇所のみを取り出したMeshを生成して返します。
        plateau::polygonMesh::Mesh filter(const plateau::polygonMesh::Mesh& src, polygonMesh::CityObjectIndex filter_id, const int uv4_atomic_index);
    };
}
