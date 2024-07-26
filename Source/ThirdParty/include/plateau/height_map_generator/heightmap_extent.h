#pragma once

namespace plateau::heightMapGenerator {
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

        double getXpercent(double pos) const {
            double val = pos - Min.x;
            return val / getXLength();
        }

        double getYpercent(double pos) const {
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
}
