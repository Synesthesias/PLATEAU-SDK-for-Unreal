
#include "heightmap_mesh_generator.h"
#include <plateau/height_map_generator/heightmap_generator.h>
//#include "libpng/png.h"

#include <plateau/geometry/geo_reference.h>
//#include <iostream>
//#include <fstream>
//#include <filesystem>

#include "hmm/base.h"
#include "hmm/heightmap.h"
//#include "hmm/stl.h"
#include "hmm/triangulator.h"


namespace plateau::heightMapMeshGenerator {

    double HeightmapMeshGenerator::getPositionFromPercent(const double percent, const double min, const double max) {
        const double dist = abs(max - min);
        const auto pos = min + (dist * percent);
        const auto clamped = std::clamp(pos, min, max);
        return clamped;
    }

    TVec2d HeightmapMeshGenerator::getPositionFromPercent(const TVec2d& percent, const TVec2d& min, const TVec2d& max) {
        return TVec2d(getPositionFromPercent(percent.x, min.x, max.x), getPositionFromPercent(percent.y, min.y, max.y));
    }
   
    plateau::polygonMesh::Mesh HeightmapMeshGenerator::generateMeshFromHeightmap(const size_t width, const size_t height, const float zScale, uint16_t* data, geometry::CoordinateSystem coordinate, TVec3d Min, TVec3d Max, TVec2f UVMin, TVec2f UVMax) {

        const float maxError = 0.001;
        const int maxTriangles = 0;
        const int maxPoints = 0;
        const float zExaggeration = 1;
        const float baseHeight = 0;

        //未使用 ====================
        const bool level = false;
        const bool invert = false;
        const int blurSigma = 0;
        const float gamma = 0;
        const int borderSize = 0;
        const float borderHeight = 1;
        const std::string normalmapPath = "";
        const std::string shadePath = "";
        const float shadeAlt = 45;
        const float shadeAz = 0;
        // =========================

        //const std::string outFile = "E:\\outfile.stl";

        //const int width = 513;
        //const int height = 513;
        std::vector<float> fdata;
        const int n = width * height;
        const float m = 1.f / 65535.f;
        fdata.resize(n);
        for (int i = 0; i < n; i++) {
            fdata[i] = data[i] * m;
        }


        const auto hm = std::make_shared<Heightmap>(width, height, fdata);
        //hm->Invert();

        Triangulator tri(hm);
        tri.Run(maxError, maxTriangles, maxPoints);

        auto points = tri.Points(zScale * zExaggeration);
        auto triangles = tri.Triangles();

        UE_LOG(LogTemp, Error, TEXT("HM2Mesh : p:%d t:%d "), points.size(), triangles.size());

        // add base
        if (baseHeight > 0) {
            const float z = -baseHeight * zScale * zExaggeration;
            AddBase(points, triangles, width, height, z);
        }

        //Export STL Debug
        //SaveBinarySTL(outFile, points, triangles);
 
        //mesh 
        
        plateau::heightMapGenerator::HeightMapExtent extent;
        extent.Min = Min;
        extent.Max = Max;
        extent.convertCoordinateFrom(coordinate);

        TVec2d UVMind(UVMin.x, UVMin.y);
        TVec2d UVMaxd(UVMax.x, UVMax.y);

        std::vector<unsigned int> indices;
        std::vector<TVec3d> vertices;
        plateau::polygonMesh::UV uv1;

        for (auto t : triangles) {
            indices.push_back((unsigned int)t.x);
            indices.push_back((unsigned int)t.y);
            indices.push_back((unsigned int)t.z);
        }

        for (auto p : points) {

            const auto& percent = TVec2d((double)p.x / (double)width, (double)p.y / (double)height);
            TVec2d position = getPositionFromPercent(percent, TVec2d(extent.Min), TVec2d(extent.Max));
            TVec3d converted = geometry::GeoReference::convertAxisFromENUTo(coordinate, TVec3d((double)position.x, (double)position.y, Min.z + (double)p.z));
            vertices.push_back(converted);

            //UV
            TVec2d uv = getPositionFromPercent(percent, UVMind, UVMaxd);
            uv1.push_back(TVec2f(uv.x, uv.y));
        }

        plateau::polygonMesh::Mesh outMesh;
        outMesh.addIndicesList(indices, 0, false);
        outMesh.addVerticesList(vertices);

        outMesh.addSubMesh("", nullptr, 0, indices.size() - 1 , 0);
        outMesh.addUV1(uv1, vertices.size());
        outMesh.addUV4WithSameVal(TVec2f(0,0), vertices.size());

        UE_LOG(LogTemp, Error, TEXT("uv size : v:%d i:%d "), vertices.size(), indices.size());

        return outMesh;
    }


} // namespace texture

