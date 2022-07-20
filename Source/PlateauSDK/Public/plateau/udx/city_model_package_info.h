#pragma once

#include <string>

/**
 * @enum PredefinedCityModelPackage
 *
 * PLATEAU標準仕様で定義済みの3D都市モデルのパッケージを表します。
 */
enum class PredefinedCityModelPackage : uint32_t {
    //! 建築物
    Building = 1u,
    //! 道路
    Road = 1u << 1,
    //! 都市計画決定情報
    UrbanPlanningDecision = 1u << 2,
    //! 土地利用
    LandUse = 1u << 3,
    //! 災害リスク
    DisasterRisk = 1u << 4,
    //! 都市設備
    UrbanFacility = 1u << 10,
    //! 植生
    Vegetation = 1u << 11,
    //! 起伏
    Relief = 1u << 12,
    //! その他()
    Unknown = 1u << 31
};

class CityModelPackageInfo {
public:
    CityModelPackageInfo(bool has_geometry, int min_lod, int max_lod)
        : has_geometry_(has_geometry), min_lod_(min_lod), max_lod_(max_lod) {
    }

    static CityModelPackageInfo getPredefined(PredefinedCityModelPackage predefined)
    {
        switch (predefined)
        {
        case PredefinedCityModelPackage::Building:
            return {true, 0, 3};
        case PredefinedCityModelPackage::DisasterRisk:
            return {true, 1, 1};
        case PredefinedCityModelPackage::LandUse:
            return { true, 1, 1 };
        case PredefinedCityModelPackage::Relief:
            return { true, 1, 3 };
        case PredefinedCityModelPackage::UrbanFacility:
            return { true, 1, 3 };
        case PredefinedCityModelPackage::Vegetation:
            return { true, 1, 3 };
        case PredefinedCityModelPackage::Road:
            return { true, 1, 3 };
        case PredefinedCityModelPackage::UrbanPlanningDecision:
            return { true, 1, 1 };
        case PredefinedCityModelPackage::Unknown:
            return {true, 0, 3};
        }
    }

    bool hasGeometry() const {
        return has_geometry_;
    }

    int minLOD() const {
        return min_lod_;
    }

    int maxLOD() const {
        return max_lod_;
    }

private:
    bool has_geometry_;
    int min_lod_;
    int max_lod_;
};
