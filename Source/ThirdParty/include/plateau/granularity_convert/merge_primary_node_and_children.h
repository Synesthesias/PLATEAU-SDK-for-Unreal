#pragma once
#include <plateau/polygon_mesh/node.h>

namespace plateau::granularityConvert {
    class MergePrimaryNodeAndChildren {
    public:
        /// 主要地物のノードとその子ノードを結合したものを、引数dst_meshに格納します。
        /// 引数に渡されるsrc_node_argはPrimaryであることを前提とします。
        void mergeWithChildren(const plateau::polygonMesh::Node& src_node_arg, plateau::polygonMesh::Mesh& dst_mesh,
                               int primary_id, int atomic_id_offset) const;
        void merge(const plateau::polygonMesh::Mesh& src_mesh, plateau::polygonMesh::Mesh& dst_mesh, const plateau::polygonMesh::CityObjectIndex& id) const;
    };
}
