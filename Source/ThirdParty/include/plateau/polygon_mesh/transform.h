#pragma once

#include <libplateau_api.h>
#include <citygml/citygml.h>
#include <plateau/polygon_mesh/quaternion.h>
#include <glm/glm.hpp>

namespace plateau::polygonMesh {
    /// ModelのNodeが持つトランスフォームです。
    class LIBPLATEAU_EXPORT Transform {
    public:
        Transform() :
            Transform(
                    TVec3d(0.0, 0.0, 0.0),
                    TVec3d(1.0, 1.0, 1.0),
                    Quaternion()) {}

        Transform(const TVec3d& local_position, const TVec3d& local_scale, const Quaternion& local_rotation) :
        local_position_(local_position),
        local_scale_(local_scale),
        local_rotation_(local_rotation){}

        TVec3d getLocalPosition() const { return local_position_; }
        TVec3d getLocalScale() const {return local_scale_; }
        Quaternion getLocalRotation() const { return local_rotation_; }
        void setLocalPosition(TVec3d local_position) { local_position_ = local_position; }
        void setLocalScale(TVec3d local_scale) { local_scale_ = local_scale; }
        void setLocalRotation(Quaternion local_rotation) { local_rotation_ = local_rotation; }

        /// GLMライブラリを使って、2つのTransformを合成したTransformを作って返します。
        /// 適用順番は自身→otherです。
        Transform apply(const Transform& other) const;

        /// 点に対してTransformを適用した点の座標を返します。
        TVec3d apply(TVec3d point) const;

        /// Transformを、GLMライブラリの変換行列にして返します。
        glm::mat4x4 toGlmMatrix() const;

        /// GLMライブラリの変換行列からTransformを作って返します。
        static Transform fromGlmMatrix(glm::mat4x4 matrix);

    private:
        TVec3d local_position_;
        TVec3d local_scale_;
        Quaternion local_rotation_;
    };
};
