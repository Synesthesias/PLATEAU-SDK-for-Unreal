#pragma once

#include <libplateau_api.h>
#include "plateau/height_map_generator/heightmap_types.h"
#include "plateau/polygon_mesh/mesh.h"

namespace plateau::heightMapGenerator {

    class LIBPLATEAU_EXPORT HeightmapMeshGenerator {
    public:
        void generateMeshFromHeightmap(
            plateau::polygonMesh::Mesh& out_mesh, const size_t width,
            const size_t height, const float zScale,
            const HeightMapElemT* data, geometry::CoordinateSystem coordinate,
            TVec3d Min, TVec3d Max, TVec2f UVMin, TVec2f UVMax, const bool invert_mesh);

    private:
        double getPositionFromPercent(double percent, double min, double max);
        TVec2d getPositionFromPercent(const TVec2d& percent, const TVec2d& min, const TVec2d& max);
    };

} // namespace plateau::texture
