#pragma once
#include <libplateau_api.h>

namespace plateau::polygonMesh {
    /// ゲームオブジェクトの向きを表すクォータニオンです。
    class LIBPLATEAU_EXPORT Quaternion {
    public:
        Quaternion() : x_(0.0), y_(0.0), z_(0.0), w_(1.0) {}
        Quaternion(double x, double y, double z, double w) : x_(x), y_(y), z_(z), w_(w) {}
        double getX() const { return x_; }
        double getY() const { return y_; }
        double getZ() const { return z_; }
        double getW() const { return w_; }
        void setX(double x) { x_ = x; }
        void setY(double y) { y_ = y; }
        void setZ(double z) { z_ = z; }
        void setW(double w) { w_ = w; }

    private:
        double x_, y_, z_, w_;
    };
}
