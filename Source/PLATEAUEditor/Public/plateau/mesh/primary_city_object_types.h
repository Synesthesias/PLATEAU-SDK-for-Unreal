#pragma once

#include <citygml/cityobject.h>

/**
 * \brief 都市モデル内の地物について「主要である」かどうかを判断するための静的メソッドを提供します。
 *
 * LOD2,3の細分化された地物ではない地物は全て「主要である」とみなされます。
 */
class PrimaryCityObjectTypes {
public:
    /**
     * \brief 主要な地物のタイプマスクを取得します。
     * \return 主要な地物のタイプマスク
     */
    static citygml::CityObject::CityObjectsType getPrimaryTypeMask() {
        return ~(
            // LOD3建築物の部品
            citygml::CityObject::CityObjectsType::COT_Door |
            citygml::CityObject::CityObjectsType::COT_Window |
            // LOD2建築物の部品
            citygml::CityObject::CityObjectsType::COT_WallSurface |
            citygml::CityObject::CityObjectsType::COT_RoofSurface |
            citygml::CityObject::CityObjectsType::COT_GroundSurface |
            citygml::CityObject::CityObjectsType::COT_ClosureSurface |
            citygml::CityObject::CityObjectsType::COT_OuterFloorSurface |
            citygml::CityObject::CityObjectsType::COT_OuterCeilingSurface |
            // LOD2,3交通
            citygml::CityObject::CityObjectsType::COT_TransportationObject
            );
    }

    /**
     * \brief 都市オブジェクトタイプが主要であるかどうかを取得します。
     * \param type 都市オブジェクトタイプ
     * \return 主要である場合true, そうでなければfalse
     */
    static bool isPrimary(citygml::CityObject::CityObjectsType type) {
        return static_cast<uint64_t>(type & getPrimaryTypeMask()) != 0ull;
    }
};
