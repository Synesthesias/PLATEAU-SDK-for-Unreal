#pragma once
#include <plateau/material_adjust/material_adjuster_common.h>
#include <citygml/cityobject.h>
#include <libplateau_api.h>

namespace plateau::materialAdjust {
    /// 地物タイプに応じてマテリアル分けをします。
    /// 使い方については MaterialAdjusterByAttr とおおむね同じで、
    /// MaterialAdjustByAttr の説明の "std::string 属性値" を "CityObjectsType タイプ" に
    /// 置き換えたものがこのクラスの説明となります。
    class LIBPLATEAU_EXPORT MaterialAdjusterByType : public MaterialAdjusterBase {
    public:
        bool registerType(const std::string& gml_id, citygml::CityObject::CityObjectsType type);
        bool registerMaterialPattern(citygml::CityObject::CityObjectsType type, int game_mat_id);
        void exec(plateau::polygonMesh::Model& model);
        bool shouldOverrideMaterial(const std::string& gml_id, int& out_next_override_game_mat_id) const override;

    private:
        std::map<std::string, citygml::CityObject::CityObjectsType> gml_id_to_type;
        std::map<citygml::CityObject::CityObjectsType, int> type_to_game_mat_id;
    };
}
