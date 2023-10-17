#pragma once

#include <libplateau_api.h>
#include <memory>
#include <plateau/polygon_mesh/mesh.h>
#include <plateau/polygon_mesh/mesh_extract_options.h>
#include <plateau/geometry/geo_coordinate.h>
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

        /**
         * CityModelから範囲内のModelを取り出します。
         */
        static std::shared_ptr<Model> extractInExtents(const citygml::CityModel& city_model, const MeshExtractOptions& options, const std::vector<plateau::geometry::Extent>& extents);

        /**
         * CityModelから範囲内のModelを取り出します。
         */
        static void extractInExtents(Model& out_model, const citygml::CityModel& city_model, const MeshExtractOptions& options, const std::vector<plateau::geometry::Extent>& extents);

        /**
         * 引数で与えられた LOD の主要地物について、次を判定して bool で返します。
         * GMLファイルからメッシュを作るとき、主要地物と [子の最小地物を結合したもの] のメッシュが同じなので、
         * 両方のメッシュを含めると重複してしまう、したがって主要地物のメッシュは含めるべきではない、という時に false を返します。
         * 主要地物のメッシュを含むべきときに true を返します。
         * 例えば、LOD2以上である建物は、次の形状が一致します。
         * ・壁、屋根などの最小地物をすべて合わせた形状
         * ・主要地物（建物1個の形状）
         * そのため、建物をインポートしたとき、主要地物は重複防止のため除外するようにします。
         * 最小地物のみから3Dモデルを構成すれば、主要地物と同じ形状を再現できるためです。
         * そのためLOD2以上の建物ではこの関数の戻り地は false です。
         * 一方で橋梁では、最小地物をすべて合わせても主要地物の形状には足りません。
         * そのため、重複があったとしても、インポートに主要地物の形状を含める必要があります。
         * そのため橋梁の関数の戻り地は true です。
         */
        static bool shouldContainPrimaryMesh(unsigned lod, const citygml::CityObject& primary_obj);
    };
}
