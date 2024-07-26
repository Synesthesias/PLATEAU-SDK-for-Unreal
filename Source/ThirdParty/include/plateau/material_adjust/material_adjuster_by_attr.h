#pragma once
#include <libplateau_api.h>
#include <plateau/polygon_mesh/model.h>
#include <plateau/material_adjust/ma_granularity.h>
#include <plateau/material_adjust/material_adjuster_common.h>
#include <map>
#include <string>

namespace plateau::materialAdjust {



    /// 属性情報に応じてマテリアル分けをします。
    /// 使い方:
    /// マテリアル分けの手がかりとして、GML ID, UV4, マテリアル分けパターンをこのクラスに渡してexecします。
    /// 具体的には:
    /// ・NodeのCityObjectListにUV4とGML IDを格納してください。
    /// ・registerAttributeで、GML IDと、検索対象の属性情報の値を登録します。検索対象の属性情報の値のパターンの数だけ実行してください。
    /// ・registerMaterialPatternで、マテリアル分けパターンである属性情報の値と、その属性値の場合に変えたいマテリアルIDを指定します。変更したいマテリアルパターンの数だけ実行してください。
    /// ・execを実行します。
    /// なお、GML IDとUV4が必要な理由は、1つのMeshの中に複数の地物があるケースでそれらを判別するためです。
    class LIBPLATEAU_EXPORT MaterialAdjusterByAttr : public MaterialAdjusterBase {
    public:
        /// 追加できた場合はtrueを返し、キーの重複により追加できなかった場合はfalseを返します。
        /// また、最小地物の属性情報には、最小地物に加えて親の主要地物と同じ属性情報を追加で登録してください。
        bool registerAttribute(const std::string& gml_id, const std::string& attr);
        /// 追加できた場合はtrueを返し、キーの重複により追加できなかった場合はfalseを返します。
        bool registerMaterialPattern(const std::string& attr, int game_mat_id);

        /// マテリアル分けを実行します。結果は引数の model に上書きで格納されます。
        void exec(plateau::polygonMesh::Model& model);

        bool containsGmlIdKey(const std::string& gml_id) const;
        bool containsAttrKey(const std::string& attr) const;

        bool shouldOverrideMaterial(const std::string& gml_id, int& out_next_override_game_mat_id) const override;


    private:
        /// GML IDと属性情報の値の辞書です。
        std::map<std::string, std::string> gml_id_to_attr;
        /// 属性情報の値と、それに対応するゲームマテリアルIDです。
        std::map<std::string, int> attr_to_game_mat_id;
    };
}
