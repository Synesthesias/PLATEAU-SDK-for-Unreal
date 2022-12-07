#pragma once

#include <string>

namespace plateau::dataset {
    /**
     * @enum PredefinedCityModelPackage
     *
     * PLATEAU標準仕様で定義済みの3D都市モデルのパッケージを表します。
     */
    enum class PredefinedCityModelPackage : uint32_t {
        None = 0,
        //! 建築物
        Building = 1u,
        //! 道路
        Road = 1u << 1,
        //! 都市計画決定情報
        UrbanPlanningDecision = 1u << 2,
        //! 土地利用
        LandUse = 1u << 3,
        //! 都市設備
        CityFurniture = 1u << 4,
        //! 植生
        Vegetation = 1u << 5,
        //! 起伏
        Relief = 1u << 6,
        //! 災害リスク
        DisasterRisk = 1u << 7,
        //! その他
        Unknown = 1u << 31
    };

    inline PredefinedCityModelPackage operator|(PredefinedCityModelPackage lhs, PredefinedCityModelPackage rhs) {
        return static_cast<PredefinedCityModelPackage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    inline PredefinedCityModelPackage operator&(PredefinedCityModelPackage lhs, PredefinedCityModelPackage rhs) {
        return static_cast<PredefinedCityModelPackage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    /**
     * \brief パッケージ種別ごとのインポート設定です。
     */
    class CityModelPackageInfo {
    public:
        CityModelPackageInfo(bool has_appearance, int min_lod, int max_lod)
            : has_appearance_(has_appearance), min_lod_(min_lod), max_lod_(max_lod) {
        }

        static CityModelPackageInfo getPredefined(PredefinedCityModelPackage predefined) {
            switch (predefined) {
            case PredefinedCityModelPackage::Building:
                return { true, 0, 3 };
            case PredefinedCityModelPackage::DisasterRisk:
                return { false, 1, 1 };
            case PredefinedCityModelPackage::LandUse:
                return { false, 1, 1 };
            case PredefinedCityModelPackage::Relief:
                return { false, 1, 3 };
            case PredefinedCityModelPackage::CityFurniture:
                return { true, 1, 3 };
            case PredefinedCityModelPackage::Vegetation:
                return { true, 1, 3 };
            case PredefinedCityModelPackage::Road:
                return { true, 1, 3 };
            case PredefinedCityModelPackage::UrbanPlanningDecision:
                return { false, 1, 1 };
            case PredefinedCityModelPackage::Unknown:
                return { true, 0, 3 };
            }
            return { false, 0, 0 };
        }

        bool hasAppearance() const {
            return has_appearance_;
        }

        int minLOD() const {
            return min_lod_;
        }

        int maxLOD() const {
            return max_lod_;
        }

    private:
        bool has_appearance_;
        int min_lod_;
        int max_lod_;
    };
}
