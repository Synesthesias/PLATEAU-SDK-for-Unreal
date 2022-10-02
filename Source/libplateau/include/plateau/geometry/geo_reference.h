#pragma once

#include <citygml/vecs.hpp>
#include "geo_coordinate.h"
#include <libplateau_api.h>

namespace plateau::geometry {

    /**
     * 座標変換の基準を保持し、座標変換します。
     *
     * zoneID についてはこちらを参照してください :
     * https://www.gsi.go.jp/sokuchikijun/jpc.html
     */
    class LIBPLATEAU_EXPORT GeoReference {
    public:
        explicit GeoReference(const TVec3d& reference_point = TVec3d(0, 0, 0), float unit_scale = 1.0,
                              CoordinateSystem coordinate_system = CoordinateSystem::ENU,
                              int zone_id = default_zone_id_);

        /**
         * 緯度・経度・高さで表現される座標を平面直角座標系に変換します。
         */
        TVec3d project(const GeoCoordinate& point) const;
        TVec3d project(const TVec3d& lat_lon) const;

        GeoCoordinate unproject(const TVec3d& point) const;

        void setReferencePoint(TVec3d point);
        TVec3d getReferencePoint() const;
        int getZoneID() const;
        void setZoneID(int value);

    private:
        TVec3d reference_point_;
        int zone_id_;
        CoordinateSystem coordinate_system_;
        float unit_scale_;

        /**
         * 関東地方で歪みの少ない座標系です。
         */
        static int constexpr default_zone_id_ = 9;
    };
}
