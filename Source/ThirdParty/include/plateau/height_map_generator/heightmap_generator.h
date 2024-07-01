#pragma once

#include <libplateau_api.h>
#include <plateau/geometry/geo_reference.h>
#include "plateau/polygon_mesh/mesh.h"
#include <plateau/height_map_generator/heightmap_extent.h>
#include <plateau/height_map_generator/triangle.h>

namespace plateau::heightMapGenerator {
    typedef uint16_t HeightmapElemT;
    typedef std::vector<HeightmapElemT> HeightmapT;

    class LIBPLATEAU_EXPORT MapWithAlpha {
    public:
        HeightmapT height_map;
        std::vector<bool> alpha_map;

        MapWithAlpha(HeightmapT height_map, std::vector<bool> alpha_map) : height_map(std::move(height_map)), alpha_map(std::move(alpha_map)) {}
    };



    class LIBPLATEAU_EXPORT HeightmapGenerator  {
    public:
        /// メイン関数です
        HeightmapT generateFromMesh(const plateau::polygonMesh::Mesh& InMesh, size_t TextureWidth, size_t TextureHeight, TVec2d margin, geometry::CoordinateSystem coordinate, bool fillEdges, TVec3d& outMin, TVec3d& outMax, TVec2f& outUVMin, TVec2f& outUVMax);

        /// extentなどがすでに決まっていることを前提にハイトマップ生成します。
        MapWithAlpha generateFromMeshAndTriangles(const plateau::polygonMesh::Mesh& in_mesh, size_t texture_width, size_t texture_height, bool fillEdges, TriangleList& triangles);

        static void savePngFile(const std::string& file_path, size_t width, size_t height, HeightmapElemT* data);
        static void saveRawFile(const std::string& file_path, size_t width, size_t height, HeightmapElemT* data);
        static HeightmapT readPngFile(const std::string& file_path, size_t width, size_t height);
        static HeightmapT readRawFile(const std::string& file_path, size_t width, size_t height);

    private:
        size_t getTileDivision(size_t triangleSize);
        double getPositionFromPercent(double percent, double min, double max);
        TVec2d getPositionFromPercent(TVec2d percent, TVec2d min, TVec2d max);
        double getHeightToPercent(double height, double min, double max);
        HeightmapElemT getPercentToGrayScale(double percent);
        TVec3d convertCoordinateFrom(geometry::CoordinateSystem coordinate, TVec3d vertice);
        bool getUVExtent(plateau::polygonMesh::UV uv, TVec2f& outMin, TVec2f& outMax);
        void applyConvolutionFilter(HeightmapElemT* image, size_t width, size_t height);
        void fillTransparentEdges(HeightmapElemT* image, const bool* alpha, size_t width, size_t height);
    };

} // namespace plateau::texture
