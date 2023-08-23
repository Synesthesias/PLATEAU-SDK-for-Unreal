#pragma once

#include <plateau/polygon_mesh/mesh.h>

namespace plateau::polygonMesh {

    /**
     * Mesh����������@�\��񋟂��܂��B
     */
    class LIBPLATEAU_EXPORT MeshMerger {
    public:
        /**
         * Mesh���}�[�W���܂��B
         */
        static void mergeMesh(
            Mesh& mesh, const Mesh& other_mesh, bool invert_mesh_front_back, bool include_textures);
    };
}
