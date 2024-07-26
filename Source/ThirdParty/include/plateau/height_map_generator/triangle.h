#pragma once

namespace plateau::heightMapGenerator {
    struct Triangle {

        TVec3d V1;
        TVec3d V2;
        TVec3d V3;

        // 2点間のベクトルを計算する関数
        TVec3d vectorBetweenPoints(const TVec3d& p1, const TVec3d& p2) {
            return {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
        }

        // 2つのベクトルの外積を計算する関数
        TVec3d crossProduct(const TVec3d& v1, const TVec3d& v2) {
            return {v1.y * v2.z - v1.z * v2.y,
                    v1.z * v2.x - v1.x * v2.z,
                    v1.x * v2.y - v1.y * v2.x};
        }

        // 平面の方程式の係数を計算する関数
        void
        planeEquationCoefficients(const TVec3d& p1, const TVec3d& p2, const TVec3d& p3, double& A, double& B, double& C,
                                  double& D) {
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

    struct TriangleList {
        std::vector<Triangle> Triangles;
        HeightMapExtent Extent;

        TriangleList(std::vector<Triangle>& triangles, HeightMapExtent& extent) :
        Triangles(triangles), Extent(extent) {
        }

        static TriangleList generateFromMesh(std::vector<unsigned  int> InIndices, std::vector<TVec3d> InVertices,
                                             geometry::CoordinateSystem coordinate) {
            HeightMapExtent extent;
            std::vector<Triangle> triangles;
            triangles.reserve(InIndices.size() / 3);
            for (size_t i = 0; i < InIndices.size(); i += 3) {

                Triangle tri;
                tri.V1 = geometry::GeoReference::convertAxisToENU(coordinate, InVertices.at(InIndices[i]));
                tri.V2 = geometry::GeoReference::convertAxisToENU(coordinate, InVertices.at(InIndices[i + 1]));
                tri.V3 = geometry::GeoReference::convertAxisToENU(coordinate, InVertices.at(InIndices[i + 2]));
                extent.setVertex(tri.V1);
                extent.setVertex(tri.V2);
                extent.setVertex(tri.V3);
                triangles.push_back(tri);
            }
            return TriangleList(triangles, extent);
        }

    };
}
