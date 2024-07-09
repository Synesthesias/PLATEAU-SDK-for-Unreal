#pragma once
#define _USE_MATH_DEFINES
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <plateau/polygon_mesh/mesh.h>
#include "longest_edge_divider_plateau.h"
#include <libplateau_api.h>

namespace plateau::heightMapAligner {

    typedef OpenMesh::TriMesh_ArrayKernelT<> MeshType;

    /// plateauのメッシュとOpenMeshのメッシュを相互変換します。
    class LIBPLATEAU_EXPORT OpenMeshConverter {
    public:
        OpenMeshConverter(float max_edge_length) : max_edge_length(max_edge_length) {}
        MeshType toOpenMesh(const plateau::polygonMesh::Mesh* mesh);
        void subdivide(MeshType& mesh);

        /// OpenMeshのメッシュをplateauのメッシュに変換します。引数のplateauMeshに対して破壊的に変更します。
        /// toOpenMesh したときと同じインスタンスを利用してください。
        void toPlateauMesh(plateau::polygonMesh::Mesh* p_mesh, const MeshType& om_mesh);

    private:
        OpenMesh::Subdivider::Uniform::GameMaterialIDPropT game_material_id_prop;
        OpenMesh::Subdivider::Uniform::UVPropT uv1_prop;
        OpenMesh::Subdivider::Uniform::UVPropT uv4_prop;
        float max_edge_length;
    };
}
