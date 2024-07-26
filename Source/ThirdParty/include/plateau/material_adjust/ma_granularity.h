#pragma once

namespace plateau::materialAdjust {
    /// マテリアル分けで使う粒度設定です。 MAはMaterialAdjustの略です。
    enum class MAGranularity {
        PerAtomic, // 最小地物単位
        PerMaterialInPrimary, // 主要地物の中でのマテリアルごと
        PerPrimary, // 主要地物単位
        CombineAll // 全結合
    };

    class MAGranularityConvert {
    public:
        /// マテリアル分け用の粒度設定を、インポート用の粒度設定に変換します。
        /// 未対応のケースがあることに注意してください。
        static plateau::polygonMesh::MeshGranularity toMeshGranularity(MAGranularity ma_granularity) {
            switch (ma_granularity) {
                case MAGranularity::PerAtomic:
                    return plateau::polygonMesh::MeshGranularity::PerAtomicFeatureObject;
                case MAGranularity::PerMaterialInPrimary:
                    throw std::runtime_error("Not implemented.");
                case MAGranularity::PerPrimary:
                    return plateau::polygonMesh::MeshGranularity::PerPrimaryFeatureObject;
                case MAGranularity::CombineAll:
                    return plateau::polygonMesh::MeshGranularity::PerCityModelArea;
                default:
                    throw std::runtime_error("Unknown MAGranularity.");
            }
        }
    };
}
