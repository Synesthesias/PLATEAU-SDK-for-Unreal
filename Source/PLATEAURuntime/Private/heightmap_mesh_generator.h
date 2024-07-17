#pragma once

#include <libplateau_api.h>
#include "plateau/polygon_mesh/mesh.h"

namespace plateau::heightMapMeshGenerator {

    class LIBPLATEAU_EXPORT HeightmapMeshGenerator  {
    public:
        plateau::polygonMesh::Mesh generateMeshFromHeightmap(const size_t width, const size_t height, const float zScale, uint16_t* data, geometry::CoordinateSystem coordinate, TVec3d Min, TVec3d Max, TVec2f UVMin, TVec2f UVMax);
        
    private:
        double getPositionFromPercent(double percent, double min, double max);
        TVec2d getPositionFromPercent(const TVec2d& percent, const TVec2d& min, const TVec2d& max);
    };

} // namespace plateau::texture
