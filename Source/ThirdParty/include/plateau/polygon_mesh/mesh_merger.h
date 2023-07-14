#pragma once

#include <plateau/polygon_mesh/mesh.h>

namespace plateau::polygonMesh {

    /**
     * Meshを結合する機能を提供します。
     */
    class LIBPLATEAU_EXPORT MeshMerger {
    public:
        /**
         * Meshをマージします。
         */
        static void mergeMesh(
            Mesh& mesh, const Mesh& other_mesh, bool invert_mesh_front_back, bool include_textures);
    };
}
