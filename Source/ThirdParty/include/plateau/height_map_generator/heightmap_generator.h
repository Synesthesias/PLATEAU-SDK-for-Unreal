#pragma once

#include <libplateau_api.h>
#include <plateau/geometry/geo_reference.h>
#include "plateau/polygon_mesh/mesh.h"
#include <plateau/height_map_generator/heightmap_extent.h>
#include <plateau/height_map_generator/triangle.h>
#include <plateau/height_map_generator/height_map_with_alpha.h>
#include <plateau/height_map_generator/heightmap_types.h>

namespace plateau::heightMapGenerator {



    class LIBPLATEAU_EXPORT HeightmapGenerator  {
    public:
        /// メイン関数です
        HeightMapT generateFromMesh(
                const plateau::polygonMesh::Mesh& InMesh, size_t TextureWidth, size_t TextureHeight,
                const TVec2d& margin, geometry::CoordinateSystem coordinate,
                bool fillEdges, bool applyConvolutionFilterForHeightMap,
                TVec3d& outMin, TVec3d& outMax, TVec2f& outUVMin, TVec2f& outUVMax);

        /// extentなどがすでに決まっていることを前提にハイトマップ生成します。
        HeightMapWithAlpha generateFromMeshAndTriangles(
                const plateau::polygonMesh::Mesh& in_mesh, size_t texture_width, size_t texture_height,
                bool fillEdges, bool applyConvolutionFilterForHeightMap,
                const TriangleList& triangles, const HeightMapT& initial_height_map);

        static void savePngFile(const std::string& file_path, size_t width, size_t height, HeightMapElemT* data);
        static void saveRawFile(const std::string& file_path, size_t width, size_t height, HeightMapElemT* data);
        static HeightMapT readPngFile(const std::string& file_path, size_t width, size_t height);
        static HeightMapT readRawFile(const std::string& file_path, size_t width, size_t height);

    private:
        size_t getTileDivision(size_t triangleSize);
        double getPositionFromPercent(double percent, double min, double max);
        TVec2d getPositionFromPercent(const TVec2d& percent, const TVec2d& min, const TVec2d& max);
        double getHeightToPercent(double height, double min, double max);
        HeightMapElemT getPercentToGrayScale(double percent);
        TVec3d convertCoordinateFrom(geometry::CoordinateSystem coordinate, TVec3d vertice);
        bool getUVExtent(plateau::polygonMesh::UV uv, TVec2f& outMin, TVec2f& outMax);
    };

} // namespace plateau::texture
