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
     * TODO
     * EPSGコードの判別と、それによって処理を変える機能は未実装です。
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
}
