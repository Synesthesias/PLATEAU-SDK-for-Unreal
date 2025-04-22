#pragma once

#include "citygml/vecs.hpp"
#include "citygml/cityobject.h"
#include <array>
#include <utility>

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
        bool containsInPolar(TVec3d point,const int epsg, bool ignore_height = true) const;
        bool containsInPolar(const citygml::CityObject& city_obj,const int epsg, bool ignore_height = true) const;

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
                    GeoCoordinate(-9999999, -9999999, -9999),
                    GeoCoordinate(9999999, 9999999, 9999)
            };
        }
    };

	/**
	 * 平面直角座標系の判定、平面直角座標の基準点取得
	 */
    struct CoordinateReferenceFactory {
        static constexpr int default_epsg = 6697;

        // EPSGとZone IDのマッピング
        static constexpr std::array<std::pair<int, int>, 13> epsg_to_zone = { {
            {10162, 1}, {10163, 2}, {10164, 3}, {10165, 4}, {10166, 5},
            {10167, 6}, {10168, 7}, {10169, 8}, {10170, 9}, {10171, 10},
            {10172, 11}, {10173, 12}, {10174, 13}
        } };

        // Zone IDごとの座標データ
        static constexpr std::array<std::pair<int, std::array<double, 3>>, 13> zone_to_point = { {
            {1, {33.0, 129.5, 0.0}}, {2, {33.0, 131.0, 0.0}}, {3, {36.0, 132.166667, 0.0}},
            {4, {33.0, 133.5, 0.0}}, {5, {36.0, 134.333333, 0.0}}, {6, {36.0, 136.0, 0.0}},
            {7, {36.0, 137.166667, 0.0}}, {8, {36.0, 138.5, 0.0}}, {9, {35.0, 139.833333, 0.0}},
            {10, {40.0, 140.833333, 0.0}}, {11, {44.0, 140.25, 0.0}}, {12, {44.0, 142.0, 0.0}},
            {13, {43.0, 144.0, 0.0}}
        } };

        // EPSGごとのzone取得
        static constexpr int GetZoneId(int epsg) {
            for (const auto& pair : epsg_to_zone) {
                if (pair.first == epsg) {
                    return pair.second;
                }
            }
            return 0;
        }

        // EPSGごとの基準点取得
        static GeoCoordinate GetReferencePoint(int epsg) {
            const int zone = GetZoneId(epsg);
            if (zone != 0) {
                for (const auto& pair : zone_to_point) {
                    if (pair.first == zone) {
                        const auto& coords = pair.second;
                        return GeoCoordinate(coords[0], coords[1], coords[2]);
                    }
                }
            }
            return GeoCoordinate();
        }

        // 極座標系・平面直角座標系判定
        // 平面直角座標系の区分についてはこちらを参照してください :
        // https://www.mlit.go.jp/plateaudocument/toc9/toc9_08/toc9_08_04/
        // “該当範囲でなければ極座標” と単純化していますが、
        // EPSG 4301(JGD2000) 等の別 CRS を誤って極座標と判定する恐れがあります。
        static bool IsPolarCoordinateSystem(int epsg) {
            return !(epsg >= 10162 && epsg <= 10174);
        }
    };
}
