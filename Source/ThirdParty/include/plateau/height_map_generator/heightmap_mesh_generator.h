#pragma once

#include <libplateau_api.h>
#include "plateau/height_map_generator/heightmap_types.h"
#include "plateau/height_map_generator/heightmap_generator.h"
#include "plateau/polygon_mesh/mesh.h"

namespace plateau::heightMapGenerator {

    class LIBPLATEAU_EXPORT HeightmapMeshGenerator : HeightmapGenerator {
    public:
        void generateMeshFromHeightmap(
            plateau::polygonMesh::Mesh& out_mesh, const size_t width,
            const size_t height, const float zScale,
            const HeightMapElemT* data, geometry::CoordinateSystem coordinate,
            TVec3d Min, TVec3d Max, TVec2f UVMin, TVec2f UVMax, const bool invert_mesh);
    };

} // namespace plateau::heightMapGenerator
