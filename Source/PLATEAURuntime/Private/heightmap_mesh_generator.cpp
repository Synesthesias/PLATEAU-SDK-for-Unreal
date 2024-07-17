
#include "heightmap_mesh_generator.h"
#include "libpng/png.h"

//#include <plateau/texture/heightmap_generator2.h>
#include <plateau/geometry/geo_reference.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "hmm/base.h"
#include "hmm/heightmap.h"
#include "hmm/stl.h"
#include "hmm/triangulator.h"


namespace plateau::heightMapMeshGenerator {
    /*
    struct HeightMapExtent {
        TVec3d Max;
        TVec3d Min;

        void setVertex(TVec3d vertex) {
            if (Max.x == 0) Max.x = vertex.x;
            if (Min.x == 0) Min.x = vertex.x;
            Max.x = std::max(Max.x, vertex.x);
            Min.x = std::min(Min.x, vertex.x);

            if (Max.y == 0) Max.y = vertex.y;
            if (Min.y == 0) Min.y = vertex.y;
            Max.y = std::max(Max.y, vertex.y);
            Min.y = std::min(Min.y, vertex.y);

            if (Max.z == 0) Max.z = vertex.z;
            if (Min.z == 0) Min.z = vertex.z;
            Max.z = std::max(Max.z, vertex.z);
            Min.z = std::min(Min.z, vertex.z);
        }

        double getXLength() const {
            return std::abs(Max.x - Min.x);
        }

        double getYLength() const {
            return std::abs(Max.y - Min.y);
        }

        double getXpercent(double pos) {
            double val = pos - Min.x;
            return val / getXLength();
        }

        double getYpercent(double pos) {
            double val = pos - Min.y;
            return val / getYLength();
        }

        TVec2d getPercent(TVec2d pos) {
            return TVec2d(getXpercent(pos.x), getYpercent(pos.y));
        }

        void convertCoordinateFrom(geometry::CoordinateSystem coordinate) {
            Max = geometry::GeoReference::convertAxisToENU(coordinate, Max);
            Min = geometry::GeoReference::convertAxisToENU(coordinate, Min);
            normalizeDirection(coordinate);
        }

        void convertCoordinateTo(geometry::CoordinateSystem coordinate) {
            Max = geometry::GeoReference::convertAxisFromENUTo(coordinate, Max);
            Min = geometry::GeoReference::convertAxisFromENUTo(coordinate, Min); 
            normalizeDirection(coordinate);
        }

        void normalizeDirection(geometry::CoordinateSystem coordinate) {
            TVec3d newMin = Min;
            TVec3d newMax = Max;    
            if (coordinate == geometry::CoordinateSystem::EUN) { 
                //Unity
                newMin = TVec3d(Min.x, Min.y, Min.z);
                newMax = TVec3d(Max.x, Max.y, Max.z);
            } else if (coordinate == geometry::CoordinateSystem::ESU) {
                //Unreal
                newMin = TVec3d(Min.x, Max.y, Min.z);
                newMax = TVec3d(Max.x, Min.y, Max.z);              
            }
            Min = newMin;
            Max = newMax;
        }
    };

    struct Triangle {

        TVec3d V1;
        TVec3d V2;
        TVec3d V3;

        // 2点間のベクトルを計算する関数
        TVec3d vectorBetweenPoints(const TVec3d& p1, const TVec3d& p2) {
            return { p2.x - p1.x, p2.y - p1.y, p2.z - p1.z };
        }

        // 2つのベクトルの外積を計算する関数
        TVec3d crossProduct(const TVec3d& v1, const TVec3d& v2) {
            return { v1.y * v2.z - v1.z * v2.y,
                    v1.z * v2.x - v1.x * v2.z,
                    v1.x * v2.y - v1.y * v2.x };
        }

        // 平面の方程式の係数を計算する関数
        void planeEquationCoefficients(const TVec3d& p1, const TVec3d& p2, const TVec3d& p3, double& A, double& B, double& C, double& D) {
            TVec3d vec1 = vectorBetweenPoints(p1, p2);
            TVec3d vec2 = vectorBetweenPoints(p1, p3);
            TVec3d normal = crossProduct(vec1, vec2);
            A = normal.x;
            B = normal.y;
            C = normal.z;
            D = -(A * p1.x + B * p1.y + C * p1.z);
        }

        // 指定された x, y 座標から z 座標を計算する関数
        double getHeight(double x, double y) {
            double A, B, C, D;
            planeEquationCoefficients(V1, V2, V3, A, B, C, D);
            return (-D - A * x - B * y) / C;
        }

        double getHeight(TVec2d vec) {
            return getHeight(vec.x, vec.y);
        }

        bool isInside(double x, double y) {
            return isInside(TVec2d(V1.x, V1.y), TVec2d(V2.x, V2.y), TVec2d(V3.x, V3.y), TVec2d(x, y));
        }

        bool isInside(TVec2d vec) {
            return isInside(TVec2d(V1), TVec2d(V2), TVec2d(V3), vec);
        }

        double crossProduct2D(const TVec2d& A, const TVec2d& B, const TVec2d& C) {
            return (B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x);
        }

        // 点Pが三角形ABCの内側にあるか判定する関数
        bool isInside(const TVec2d& A, const TVec2d& B, const TVec2d& C, const TVec2d& P) {
            double crossABP = crossProduct2D(A, B, P);
            double crossBCP = crossProduct2D(B, C, P);
            double crossCAP = crossProduct2D(C, A, P);

            // 点Pが３角形ABCの内側にあるか判定
            if ((crossABP >= 0 && crossBCP >= 0 && crossCAP >= 0) ||
                (crossABP <= 0 && crossBCP <= 0 && crossCAP <= 0)) {
                return true;
            }
            return false;
        }

        // 2つの線分が交差しているか判定する関数
        bool segmentsIntersect(const TVec2d& p1, const TVec2d& p2, const TVec2d& p3, const TVec2d& p4) {
            double cp1 = crossProduct2D(p1, p2, p3);
            double cp2 = crossProduct2D(p1, p2, p4);
            double cp3 = crossProduct2D(p3, p4, p1);
            double cp4 = crossProduct2D(p3, p4, p2);

            // 交差する条件：各線分の両側に、他方の両端点が存在する
            return ((cp1 > 0 && cp2 < 0) || (cp1 < 0 && cp2 > 0)) && ((cp3 > 0 && cp4 < 0) || (cp3 < 0 && cp4 > 0));
        }

        // 2つの点の中点を計算する関数
        TVec3d midpoint(const TVec3d& p1, const TVec3d& p2) {
            TVec3d mid;
            mid.x = (p1.x + p2.x) / 2.0;
            mid.y = (p1.y + p2.y) / 2.0;
            mid.z = (p1.z + p2.z) / 2.0;
            return mid;
        }

        TVec3d getCenter() {
            TVec3d mid1 = midpoint(V1, V2);
            TVec3d mid2 = midpoint(V2, V3);
            return midpoint(mid1, mid2);
        }
    };

    struct Tile {
        TVec2d Max;
        TVec2d Min;
        int ID;
        std::shared_ptr<std::vector<Triangle>> Triangles;

        Tile(int id, TVec2d min, TVec2d max){
            ID = id;
            Min = min;
            Max = max;
            Triangles = std::make_shared<std::vector<Triangle>>();
        }

        void getCornerPoints(TVec2d& p1, TVec2d& p2, TVec2d& p3, TVec2d& p4) const {
            p1 = Min;
            p2 = TVec2d(Min.x, Max.y);
            p3 = TVec2d(Max.x, Min.y);
            p4 = Max;
        }

        // pointが範囲内にあるかどうかを判定する関数
        bool isWithin(const TVec2d& point) const {
            return (point.x >= Min.x && point.x <= Max.x && point.y >= Min.y && point.y <= Max.y);
        }

        bool isAlmostWithin(const TVec2d& point, const double toleranceMargin) const {
            return (point.x >= Min.x - toleranceMargin && point.x <= Max.x + toleranceMargin && point.y >= Min.y - toleranceMargin && point.y <= Max.y + toleranceMargin);
        }
    };

    // MeshからHeightMap画像となる16bitグレースケール配列を生成し、適用範囲となる位置を計算します
    // ENUに変換してから処理を実行し、元のCoordinateに変換して値を返します
    std::vector<uint16_t> HeightmapGenerator::generateFromMesh(
        const plateau::polygonMesh::Mesh& InMesh, size_t TextureWidth, size_t TextureHeight, TVec2d margin, 
        geometry::CoordinateSystem coordinate, bool fillEdges, TVec3d& outMin, TVec3d& outMax, TVec2f& outUVMin, TVec2f& outUVMax) {

        const auto& InVertices = InMesh.getVertices();
        const auto& InIndices = InMesh.getIndices();

        HeightMapExtent extent;
        std::vector<Triangle> triangles;
        for (size_t i = 0; i < InIndices.size(); i += 3) {

            Triangle tri;
            tri.V1 = convertCoordinateFrom(coordinate, InVertices.at(InIndices[i]));
            tri.V2 = convertCoordinateFrom(coordinate, InVertices.at(InIndices[i + 1]));
            tri.V3 = convertCoordinateFrom(coordinate, InVertices.at(InIndices[i + 2]));
            extent.setVertex(tri.V1);
            extent.setVertex(tri.V2);
            extent.setVertex(tri.V3);
            triangles.push_back(tri);
        }

        //UV
        if (!getUVExtent(InMesh.getUV1(), outUVMin, outUVMax)) {
            //UV情報が取得できなかった場合 0,1に設定
            outUVMin.x = outUVMin.y = 0.f;
            outUVMax.x = outUVMax.y = 1.f;
        }
        else {
            //UV情報が取得できた場合、UVに余白を追加
            if (margin.x != 0 || margin.y != 0) {
                auto uvSize = TVec2f(outUVMax.x - outUVMin.x, outUVMax.y - outUVMin.y);
                auto baseExtentSize = TVec2d(extent.getXLength() / uvSize.x, extent.getYLength() / uvSize.y);
                auto marginPercent = TVec2f(margin.x / baseExtentSize.x, margin.y / baseExtentSize.y);
                outUVMax.x += marginPercent.x;
                outUVMax.y += marginPercent.y;
            }
        }

        //Extentに余白を追加
        extent.Max.x += margin.x;
        extent.Max.y += margin.y;

        //Tile生成 : Triangleのイテレーションを減らすため、グリッド分割して範囲ごとに処理します
        std::vector<Tile> tiles;
        size_t division = getTileDivision(triangles.size()); //縦横のグリッド分割数
        double xTiles = TextureWidth / division;
        double yTiles = TextureHeight / division;
        int tileIndex = 0;
        TVec2d prev(0, 0);

        for (double y = yTiles; y <= TextureHeight; y += yTiles) {
            for (double x = xTiles; x <= TextureWidth; x += xTiles) {

                TVec2d min = prev;
                TVec2d max(x, y);
                Tile tile(tileIndex++, min, max);
                prev.x = x;
                if (x + xTiles > TextureWidth) {
                    tile.Max.x = TextureWidth;
                    prev.x = 0;
                }
                if(y + yTiles > TextureHeight)
                    tile.Max.y = TextureHeight;
                tiles.push_back(tile);

                /**　タイル内に含まれるTriangleをセット 
                * 四角タイル内に三角メッシュの頂点が含まれるか
                * 三角メッシュ内に四角タイルの頂点が含まれるか
                * 四角タイル、三角メッシュの各辺が交差するか
                * の3点について検証します
                */
/*
                for (auto tri : triangles) {
                    //3次元座標をテクスチャ上の座標に変換してTriangleがTile範囲内にあるかチェック   
                    TVec2d tmin(0, 0);
                    TVec2d tmax(TextureWidth, TextureHeight);
                    //Triangle頂点(Texture座標）
                    const auto& v1 = getPositionFromPercent(extent.getPercent(TVec2d(tri.V1)), tmin, tmax);
                    const auto& v2 = getPositionFromPercent(extent.getPercent(TVec2d(tri.V2)), tmin, tmax);
                    const auto& v3 = getPositionFromPercent(extent.getPercent(TVec2d(tri.V3)), tmin, tmax);
                    const double tolerance = xTiles / 2;
                    if (tile.isWithin(v1) || tile.isWithin(v2) || tile.isWithin(v3)) {
                        tile.Triangles->push_back(tri);
                        continue;
                    }

                    //Tile頂点(Texture座標）
                    TVec2d r1, r2, r3, r4;
                    tile.getCornerPoints(r1, r2, r3, r4);

                    //Triangleをテクスチャ上の座標に変換してTile頂点がTriangle範囲内にあるかチェック
                    Triangle TexTri{ TVec3d(v1.x,v1.y), TVec3d(v2.x,v2.y), TVec3d(v3.x, v3.y) };
                    if (TexTri.isInside(r1) || TexTri.isInside(r2) || TexTri.isInside(r3) || TexTri.isInside(r4)) {
                        tile.Triangles->push_back(tri);
                        continue;
                    }

                    // 3角形と4角形の各辺が交差しているかどうかを判定(Texture座標）
                    if (tri.segmentsIntersect(v1, v2, r1, r2) || tri.segmentsIntersect(v1, v2, r2, r3) || tri.segmentsIntersect(v1, v2, r3, r4) ||
                        tri.segmentsIntersect(v2, v3, r1, r2) || tri.segmentsIntersect(v2, v3, r2, r3) || tri.segmentsIntersect(v2, v3, r3, r4) ||
                        tri.segmentsIntersect(v3, v1, r1, r2) || tri.segmentsIntersect(v3, v1, r2, r3) || tri.segmentsIntersect(v3, v1, r3, r4)) {
                        tile.Triangles->push_back(tri);
                        continue;
                    }
                }
            }
            prev.y = (y < TextureHeight) ? y : 0;
        }

        int TextureDataSize = TextureWidth * TextureHeight;

        // Initialize Texture Data Array
        std::unique_ptr<uint16_t[]> TextureData = std::make_unique<uint16_t[]>(TextureDataSize);
        std::unique_ptr<bool[]> AlphaData = std::make_unique<bool[]>(TextureDataSize);

        size_t index = 0;
        for (int y = TextureHeight - 1; y >= 0; y--) {
            for (int x = 0; x < TextureWidth; x++) {

                const auto& percent = TVec2d((double)x / (double)TextureWidth, (double)y / (double)TextureHeight);
                const auto& p = getPositionFromPercent(percent, TVec2d(extent.Min), TVec2d(extent.Max));

                uint16_t GrayValue = 0;
                bool ValueSet = false;
                for (auto& tile : tiles) {
                    if (tile.isWithin(TVec2d(x, y))) { 
                        for (auto tri : *tile.Triangles) {
                            if (tri.isInside(p)) {
                                double Height = tri.getHeight(p);
                                double HeightPercent = getHeightToPercent(Height, extent.Min.z, extent.Max.z);
                                GrayValue = getPercentToGrayScale(HeightPercent);
                                ValueSet = true;
                                break;
                            }
                        }
                        if(ValueSet) break;   
                    }
                    
                }
                if (index < TextureDataSize) {
                    TextureData[index] = GrayValue;
                    AlphaData[index] = ValueSet;
                }
                    
                index++;
            }
        }
        
        //透明部分をエッジ色でFill
        if(fillEdges)
            fillTransparentEdges(TextureData.get(), AlphaData.get(), TextureWidth, TextureHeight);

        //平滑化
        applyConvolutionFilter(TextureData.get(), TextureWidth, TextureHeight);

        std::vector<uint16_t> heightMapData(TextureWidth * TextureHeight);
        memcpy(heightMapData.data(), TextureData.get(), sizeof(uint16_t) * TextureDataSize);

        extent.convertCoordinateTo(coordinate);
        outMin = extent.Min;
        outMax = extent.Max;

        return heightMapData;
    }

    //Mesh数に応じてTile分割数を決めます
    size_t HeightmapGenerator::getTileDivision(size_t triangleSize) {
        if (triangleSize > 1000000)
            return 18;
        if (triangleSize > 100000)
            return 16;
        if (triangleSize > 10000)
            return 12;
        return 8;
    }

    double HeightmapGenerator::getPositionFromPercent(double percent, double min, double max) {
        double dist = abs(max - min);
        return min + (dist * percent);
    }

    TVec2d HeightmapGenerator::getPositionFromPercent(TVec2d percent, TVec2d min, TVec2d max) {
        return TVec2d(getPositionFromPercent(percent.x, min.x, max.x), getPositionFromPercent(percent.y, min.y, max.y));
    }

    double HeightmapGenerator::getHeightToPercent(double height, double min, double max) {
        double dist = abs(max - min);
        double height_in_dist = abs(height - min);
        return height_in_dist / dist;
    }

    uint16_t HeightmapGenerator::getPercentToGrayScale(double percent) {
        uint16_t size = 65535;
        return static_cast<uint16_t>(static_cast<double>(size) * percent);
    }

    TVec3d HeightmapGenerator::convertCoordinateFrom(geometry::CoordinateSystem coordinate, TVec3d vertice) {
        return geometry::GeoReference::convertAxisToENU(coordinate, vertice);
    }

    void HeightmapGenerator::applyConvolutionFilter(uint16_t* image, const size_t width, const size_t height) {
        size_t imageSize = width * height;
        std::unique_ptr<uint16_t[]> tempImage = std::make_unique<uint16_t[]>(imageSize);
        memcpy(tempImage.get(), image, sizeof(uint16_t) * imageSize);
        //エッジを除外して処理
        for (size_t y = 1; y < height - 1; ++y) { 
            for (size_t x = 1; x < width - 1; ++x) {
                // 3x3の領域のピクセル値の平均を計算
                int sum = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        sum += image[(y + dy) * width + (x + dx)];
                    }
                }
                tempImage[y * width + x] = sum / 9; 
            }
        }
        memcpy(image, tempImage.get(), sizeof(uint16_t) * imageSize);
    }

    //UVの最大、最小値を取得します。値が取得できなかった場合はfalseを返します。
    bool HeightmapGenerator::getUVExtent(plateau::polygonMesh::UV uvs, TVec2f& outMin, TVec2f& outMax) {
        TVec2f Min, Max;
        for (auto uv : uvs) {
            if (Max.x == 0) Max.x = uv.x;
            if (Min.x == 0) Min.x = uv.x;
            Max.x = std::max(Max.x, uv.x);
            Min.x = std::min(Min.x, uv.x);

            if (Max.y == 0) Max.y = uv.y;
            if (Min.y == 0) Min.y = uv.y;
            Max.y = std::max(Max.y, uv.y);
            Min.y = std::min(Min.y, uv.y);
        }
        outMin = Min;
        outMax = Max;
        return !(outMin.x == 0.f && outMin.y == 0.f && outMax.x == 0.f && outMax.y == 0.f);
    }

    // 16bitグレースケールのpng画像を保存します
    void HeightmapGenerator::savePngFile(const std::string& file_path, size_t width, size_t height, uint16_t* data) {

#ifdef WIN32
        const auto regular_name = std::filesystem::u8path(file_path).wstring();
        FILE* fp;
        _wfopen_s(&fp, regular_name.c_str(), L"wb");
#else
        FILE* fp = fopen(file_path.c_str(), "wb");
#endif       
        if (!fp) {
            throw std::runtime_error("Error: Failed to open PNG file for writing.");
        }

        png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(fp);
            throw std::runtime_error("Error: png_create_write_struct failed.");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            fclose(fp);
            png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
            throw std::runtime_error("Error: png_create_info_struct failed.");
        }
        png_init_io(png_ptr, fp);
        png_set_IHDR(png_ptr, info_ptr, width, height,
            16, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_write_info(png_ptr, info_ptr);

        png_bytep row = (png_bytep)malloc(width * sizeof(png_uint_16));
        if (!row) {
            fclose(fp);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            throw std::runtime_error("Error: Failed to allocate memory for PNG row.");
        }

        int index = 0;
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                png_uint_16 value = (png_uint_16)data[index];
                index++;
                row[x * 2] = (png_byte)(value >> 8);
                row[x * 2 + 1] = (png_byte)(value & 0xFF);
            }
            png_write_row(png_ptr, row);
        }

        free(row);
        png_write_end(png_ptr, NULL);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
    }

    // PNG画像を読み込み、グレースケールの配列を返します
    std::vector<uint16_t> HeightmapGenerator::readPngFile(const std::string& file_path, size_t width, size_t height) {

#ifdef WIN32
        const auto regular_name = std::filesystem::u8path(file_path).wstring();
        FILE* fp;
        _wfopen_s(&fp, regular_name.c_str(), L"rb");
#else
        FILE* fp = fopen(file_path.c_str(), "rb");
#endif
        if (!fp) {
            throw std::runtime_error("Error: Unable to open file for reading.");
        }

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            fclose(fp);
            throw std::runtime_error("Error: Failed to create PNG read struct.");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            fclose(fp);
            throw std::runtime_error("Error: Failed to create PNG info struct.");
        }

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 0);
        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        int color_type = png_get_color_type(png_ptr, info_ptr);

        if (bit_depth != 16 || color_type != PNG_COLOR_TYPE_GRAY) {
            throw std::runtime_error("Error: Invalid PNG format. Expected 16-bit grayscale.");
        }

        std::vector<uint16_t> grayscaleData(width * height);

        png_bytepp row_pointers = (png_bytepp)malloc(sizeof(png_bytep) * height);
        for (size_t y = 0; y < height; ++y) {
            row_pointers[y] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
        }

        png_read_image(png_ptr, row_pointers);

        for (size_t y = 0; y < height; ++y) {
            png_bytep row = row_pointers[y];
            for (int x = 0; x < width; ++x) {
                grayscaleData[y * width + x] = (row[x * 2] << 8) + row[x * 2 + 1];
            }
        }

        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);

        return grayscaleData;
    }

    // 16bitグレースケールのpng画像を保存します   
    void HeightmapGenerator::saveRawFile(const std::string& file_path, size_t width, size_t height, uint16_t* data) {

#ifdef WIN32
        const auto regular_name = std::filesystem::u8path(file_path).wstring();
#else
        const auto regular_name = std::filesystem::u8path(file_path).u8string();
#endif

        std::ofstream outputFile(regular_name, std::ios::out | std::ios::binary);

        if (!outputFile) {
            throw std::runtime_error("Error: Unable to open file for writing.");
            return;
        }

        // Write image data
        for (int i = 0; i < width * height; ++i) {
            uint16_t pixelValue = data[i];
            outputFile.write(reinterpret_cast<const char*>(&pixelValue), sizeof(uint16_t));
        }

        outputFile.close();
    }

    // Raw画像を読み込み、グレースケールの配列を返します
    std::vector<uint16_t> HeightmapGenerator::readRawFile(const std::string& file_path, size_t width, size_t height) {

#ifdef WIN32
        const auto regular_name = std::filesystem::u8path(file_path).wstring();
#else
        const auto regular_name = std::filesystem::u8path(file_path).u8string();
#endif

        std::ifstream file(regular_name, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Error: Unable to open file for reading. ");
        }

        std::vector<uint16_t> grayscaleData(width * height);
        // データをバイナリとして読み込む
        file.read(reinterpret_cast<char*>(&grayscaleData[0]), width * height * sizeof(uint16_t));
        file.close();

        return grayscaleData;
    }

    void HeightmapGenerator::fillTransparentEdges(uint16_t* image, const bool* alpha, const size_t width, const size_t height) {
        struct Pixel {
            int x, y;
            uint16_t color;
        };
        std::vector<Pixel> edgePixels;

        // エッジを検索
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (alpha[y * width + x]) {
                    edgePixels.push_back({ x, y, image[y * width + x] });
                    break;
                }
            }
            for (int x = width - 1; x >= 0; --x) {
                if (alpha[y * width + x]) {
                    edgePixels.push_back({ x, y, image[y * width + x] });
                    break;
                }
            }
        }
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                if (alpha[y * width + x]) {
                    edgePixels.push_back({ x, y, image[y * width + x] });
                    break;
                }
            }
            for (int y = height - 1; y >= 0; --y) {
                if (alpha[y * width + x]) {
                    edgePixels.push_back({ x, y, image[y * width + x] });
                    break;
                }
            }
        }

        // アルファを埋める
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                if (!alpha[y * width + x]) {
                    // 最も近い端のピクセルを探す
                    double minDist = std::numeric_limits<double>::max();
                    uint16_t nearestColor = 0;
                    for (const auto& p : edgePixels) { 
                        double dist = std::sqrt((p.x - x) * (p.x - x) + (p.y - y) * (p.y - y));
                        if (dist < minDist) {
                            minDist = dist;
                            nearestColor = p.color;
                        }
                    }
                    image[y * width + x] = nearestColor;
                }
            }
        }
    }
    */
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

        const std::string outFile = "E:\\outfile.stl";

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
        SaveBinarySTL(outFile, points, triangles);


        //mesh 

        HeightMapExtent extent;
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

