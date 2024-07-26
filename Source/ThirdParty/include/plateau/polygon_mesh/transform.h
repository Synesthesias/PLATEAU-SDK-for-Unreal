#pragma once

#include <libplateau_api.h>
#include <citygml/vecs.hpp>
#include <memory>

namespace plateau::meshWriter {
    class TransformStack;
}
namespace plateau::polygonMesh {
    class Quaternion;
}


namespace plateau::polygonMesh {
    /// ModelのNodeが持つトランスフォームです。
    class LIBPLATEAU_EXPORT Transform {
    public:
        Transform();
        Transform(const TVec3d& local_position,
                  const TVec3d& local_scale,
                  const Quaternion& local_rotation);

        TVec3d getLocalPosition() const;
        TVec3d getLocalScale() const;
        Quaternion getLocalRotation() const;
        void setLocalPosition(TVec3d local_position);
        void setLocalScale(TVec3d local_scale);
        void setLocalRotation(Quaternion local_rotation);

        /// GLMライブラリを使って、2つのTransformを合成したTransformを作って返します。
        /// 適用順番は自身→otherです。
        Transform apply(const Transform& other) const;

        /// 点に対してTransformを適用した点の座標を返します。
        TVec3d apply(TVec3d point) const;

        // Implは実質internalのように扱いたいためのfriendです
        friend class plateau::meshWriter::TransformStack;

    private:
        class Impl;
        std::shared_ptr<Impl> impl_;
    };
};
