#pragma once

#include <libplateau_api.h>
#include <memory>
#include <plateau/polygon_mesh/mesh.h>
#include <plateau/polygon_mesh/mesh_extract_options.h>
#include "citygml/citymodel.h"
#include "model.h"

namespace plateau::polygonMesh {

    /**
     * CityModelからModel(メッシュ等)を構築します。
     * このクラスの利用者である各ゲームエンジンは、このクラスから受け取った Model を元に
     * ゲームオブジェクト、メッシュ、テクスチャを生成することが期待されます。
     *
     * 詳しくは Model クラスのコメントを参照してください。
     */
    class LIBPLATEAU_EXPORT MeshExtractor {
    public:

        /**
         * CityModel から Modelを取り出します。
         * Model を new して shared_ptr で返します。
         */
        static std::shared_ptr<Model> extract(const citygml::CityModel& city_model, const MeshExtractOptions& options);

        /**
         * extract関数について、戻り値がスマートポインタの代わりに、引数にデータを追加するようになった版です。
         * DLL利用者との間でModelをやりとりするには生ポインタである必要があるための措置です。
         * 別途 初期化されたばかりのModelを引数で受け取り、そのModelに対して結果を格納します。
         * 生ポインタのdeleteはDLLの利用者の責任です。
         */
        static void extract(Model& out_model, const citygml::CityModel& city_model, const MeshExtractOptions& options);
    };
}
