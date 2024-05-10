#pragma once

#include <libplateau_api.h>
#include "plateau/polygon_mesh/mesh.h"

namespace plateau::texture {

    class HeightmapGenerator  {
    public:

        std::vector<uint16_t> generateFromMesh(const plateau::polygonMesh::Mesh& InMesh, size_t TextureWidth, size_t TextureHeight, TVec2d margin, geometry::CoordinateSystem coordinate, TVec3d& outMin, TVec3d& outMax, TVec2f& outUVMin, TVec2f& outUVMax);
        static void savePngFile(const std::string& file_path, size_t width, size_t height, uint16_t* data);
        static void saveRawFile(const std::string& file_path, size_t width, size_t height, uint16_t* data);
        static std::vector<uint16_t> readPngFile(const std::string& file_path, size_t width, size_t height);
        static std::vector<uint16_t> readRawFile(const std::string& file_path, size_t width, size_t height);

    private:
        double getPositionFromPercent(double percent, double min, double max);
        double getHeightToPercent(double height, double min, double max);
        uint16_t getPercentToGrayScale(double percent);
        TVec3d convertCoordinateFrom(geometry::CoordinateSystem coordinate, TVec3d vertice);
        bool getUVExtent(plateau::polygonMesh::UV uv, TVec2f& outMin, TVec2f& outMax);
    };

} // namespace plateau::texture
