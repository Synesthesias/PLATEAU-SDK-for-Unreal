#pragma once

#include "citygml/vecs.hpp"
#include "citygml/cityobject.h"


namespace plateau::geometry {

    /**
     * 緯度・経度・高さ による位置表現です。
     * PLATEAU の gmlファイルでは 緯度・経度・高さ によって座標が表現されます。
     *
     * 厳密にどの基準に基づいた座標であるかは、
     * GMLファイル名 または CityModel の Envelope の srcName に記載された
     * EPSGコードによって判別できます。
     * EPSGコードが 6697 のとき、それは
     * 「日本測地系2011における経緯度座標系と東京湾平均海面を基準とする標高の複合座標参照系」
     * になります。
     * EPSGコードが 10162 ～ 10174　の場合は平面直角座標系となります。
     */
    struct GeoCoordinate {
        double latitude;
        double longitude;
        double height;

        GeoCoordinate() = default;

        GeoCoordinate(double lat, double lon, double height) :
                latitude(lat),
                longitude(lon),
                height(height) {
        }

        GeoCoordinate operator+(GeoCoordinate op) const;
        GeoCoordinate operator*(double op) const;
        GeoCoordinate operator-(GeoCoordinate op) const;
        GeoCoordinate operator/(GeoCoordinate op) const;
    };


    /**
     * @enum CoordinateSystem
     *
     * 各列挙子について、3つのアルファベットはXYZ軸がどの方角、方向になるかを表しています。<br/>
     * N,S,E,Wはそれぞれ北,南,東,西<br/>
     * U,Dはそれぞれ上,下<br/>
     * に対応します。<br/>
     */
    enum class CoordinateSystem {
        //! PLATEAUでの座標系
        ENU = 0,
        WUN = 1,
        //! Unreal Engineでの座標系
        ESU = 2,
        //! Unityでの座標系
        EUN = 3
    };


    /**
     * 緯度・経度・高さの最小・最大で表現される範囲です。
     */
    struct Extent {
        GeoCoordinate min;
        GeoCoordinate max;

        Extent(const GeoCoordinate& min, const GeoCoordinate& max) {
            this->min = min;  // NOLINT(cppcoreguidelines-prefer-member-initializer)
            this->max = max;  // NOLINT(cppcoreguidelines-prefer-member-initializer)
        }

        bool contains(GeoCoordinate point, bool ignore_height = true) const;
        bool contains(TVec3d point, bool ignore_height = true) const;

        /**
         * 引数 city_obj の位置を推定し、その位置が Extent の範囲内に含まれるかどうかを返します。
         * city_obj の位置が不明の場合は false を返します。
         */
        bool contains(const citygml::CityObject& city_obj, bool ignore_height = true) const;

        /**
         * 平面直角座標系の判定を含む処理です
         * 平面直角座標の場合はunprojectして緯度経度に変換してから判定します。
         */
        bool containsInPolar(TVec3d point,const double epsg, bool ignore_height = true) const;
        bool containsInPolar(const citygml::CityObject& city_obj,const double epsg, bool ignore_height = true) const;

        /**
         * other と交わる箇所があるかどうかを返します。
         * ただし other の高さは無視して緯度と経度の2次元のみで判定します。
         */
        bool intersects2D(const Extent& other) const;

        /**
         * min と max の中点を GeoCoordinate で返します。
         */
        GeoCoordinate centerPoint() const;

        /// Extentの南西端のUVを(0,0),北東端のUVを(1,1)とするとき、指定位置のUVを求めます。
        TVec2f uvAt(GeoCoordinate coord) const;

        static Extent all() {
            return {
                    GeoCoordinate(-90, -180, -9999),
                    GeoCoordinate(90, 180, 9999)
            };
        }
    };

    /**
    * 平面直角座標判定、平面直角座標の基準点取得
    */
    struct CoordinateReferenceFactory {
        // EPSGごとのzone取得
        static int GetZoneId(double epsg) {
            // 日本測地系2011（JGD2011）に基づく平面直角座標系
            if (epsg == 10162) {
                return 1; // 1系
            }
            else if (epsg == 10163) {
                return 2; // 2系
            }
            else if (epsg == 10164) {
                return 3; // 3系
            }
            else if (epsg == 10165) {
                return 4; // 4系
            }
            else if (epsg == 10166) {
                return 5; // 5系
            }
            else if (epsg == 10167) {
                return 6; // 6系
            }
            else if (epsg == 10168) {
                return 7; // 7系
            }
            else if (epsg == 10169) {
                return 8; // 8系
            }
            else if (epsg == 10170) {
                return 9; // 9系
            }
            else if (epsg == 10171) {
                return 10; // 10系
            }
            else if (epsg == 10172) {
                return 11; // 11系
            }
            else if (epsg == 10173) {
                return 12; // 12系
            }
            else if (epsg == 10174) {
                return 13; // 13系
            }
            return 0;
        }

        // EPSGごとの基準点取得
        static GeoCoordinate GetReferencePoint(double epsg) {
            const int zone = GetZoneId(epsg);
            if (zone != 0)
                return GetReferencePointByZone(zone);
            return GeoCoordinate();
        }

        // Zone IDごとの基準点
        // zoneに紐づく基準点はPolarToPlaneCartesianにハードコードで持っているが値が取得できないので、ここで定義
        static GeoCoordinate GetReferencePointByZone(int zone_id) {
            switch (zone_id) {
            case 1:
                return GeoCoordinate(33, 129.5, 0);
            case 2:
                return GeoCoordinate(33, 131, 0);
            case 3:
                return GeoCoordinate(36, 132.166667, 0);
            case 4:
                return GeoCoordinate(33, 133.5, 0);
            case 5:
                return GeoCoordinate(36, 134.333333, 0);
            case 6:
                return GeoCoordinate(36, 136, 0);
            case 7:
                return GeoCoordinate(36, 137.166667, 0);
            case 8:
                return GeoCoordinate(36, 138.5, 0);
            case 9:
                return GeoCoordinate(35, 139.833333, 0);
            case 10:
                return GeoCoordinate(40, 140.833333, 0);
            case 11:
                return GeoCoordinate(44, 140.25, 0);
            case 12:
                return GeoCoordinate(44, 142, 0);
            case 13:
                return GeoCoordinate(43, 144, 0);
            case 14:
                return GeoCoordinate(26, 142, 0);
            case 15:
                return GeoCoordinate(26, 127.5, 0);
            case 16:
                return GeoCoordinate(24, 124, 0);
            case 17:
                return GeoCoordinate(31, 131, 0);
            case 18:
                return GeoCoordinate(20, 136, 0);
            case 19:
                return GeoCoordinate(25, 154, 0);
            default:
                return GeoCoordinate();
            }
        }

        // 極座標系・平面直角座標系判定
        static bool IsPolarCoordinateSystem(double epsg) {
            // 平面直角座標系の区分についてはこちらを参照してください :
            // https://www.mlit.go.jp/plateaudocument/toc9/toc9_08/toc9_08_04/
            if (epsg >= 10162 && epsg <= 10174) {
                return false;
            }
            return true;
        }
    };
}
