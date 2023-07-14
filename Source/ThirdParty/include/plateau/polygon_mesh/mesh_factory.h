#pragma once

#include <plateau/polygon_mesh/mesh.h>
#include <plateau/polygon_mesh/mesh_extract_options.h>
#include <citygml/polygon.h>
#include <citygml/cityobject.h>
#include <list>
#include "plateau/geometry/geo_reference.h"

namespace plateau::polygonMesh {

    /**
     * citygml::Polygon(CityGMLパーサーの出力)をpolygonMesh::Mesh(3Dモデルデータ)に変換する機能を提供します。
     */
    class LIBPLATEAU_EXPORT MeshFactory {
    public:
        MeshFactory(
            std::unique_ptr<Mesh>&& target = nullptr,
            const MeshExtractOptions& mesh_extract_options = MeshExtractOptions(),
            const geometry::GeoReference& geo_reference = geometry::GeoReference(9));

        std::unique_ptr<Mesh> releaseMesh() {
            return std::move(mesh_);
        }

        /**
         * citygml::Polygon の情報を Mesh 向けに変換し、 引数の mesh に書き加えます。
         * 引数で与えられたポリゴンのうち、次の情報を追加します。
         * ・頂点リスト、インデックスリスト、UV1、テクスチャ。
         * なおその他の情報のマージには未対応です。例えば LinearRing は考慮されません。
         * options.export_appearance の値によって、 mergeWithTexture または mergeWithoutTexture を呼び出します。
         */
        void addPolygon(const citygml::Polygon& polygon, const std::string& gml_path) const;
        
        /**
         * 主要地物の主要地物IDを設定しMeshをマージします。
         */
        void addPolygonsInPrimaryCityObject(
            const citygml::CityObject& city_object, unsigned lod,
            const std::string& gml_path);

        /**
         * 最小地物のMeshのuv4フィールドに最小地物IDを設定するために、最小地物を構成するPolygonに含まれるVertex数を取得します。
         */
        static long long countVertices(const citygml::CityObject& city_object, unsigned int lod);

        /**
         * 最小地物に含まれるすべてのポリゴンをメッシュに追加します。
         */
        void addPolygonsInAtomicCityObject(
            const citygml::CityObject& parent_city_object,
            const citygml::CityObject& city_object,
            const unsigned lod, const std::string& gml_path);

        /**
         * 最小地物に含まれるすべてのポリゴンをメッシュに追加します。
         */
        void addPolygonsInAtomicCityObjects(
            const citygml::CityObject& parent_city_object,
            const std::list<const citygml::CityObject*>& city_objects,
            unsigned lod, const std::string& gml_path);

        /**
         * city_obj に含まれるポリゴンをすべて検索し、リストで返します。
         * 子の CityObject は検索しません。
         * 子の Geometry は再帰的に検索します。
         */
        static void findAllPolygons(const citygml::CityObject& city_obj, unsigned lod, std::list<const citygml::Polygon*>& out_polygons, long long& out_vertices_count, const plateau::geometry::Extent& extent = plateau::geometry::Extent::all());

        /**
         * PLATEAUからメッシュを読み込んで座標軸を変換をするとき、このままだとメッシュが裏返ることがあります（座標軸が反転したりするので）。
         * 裏返りを補正する必要があるかどうかを bool で返します。
         */
        static bool shouldInvertIndicesOnMeshConvert(plateau::geometry::CoordinateSystem sys);
    private:
        MeshExtractOptions options_;
        geometry::GeoReference geo_reference_;

        std::unique_ptr<Mesh> mesh_;
        // 新規に主要地物を追加する際に利用可能なインデックス
        CityObjectIndex available_primary_index_;
        // 主要地物のgml::idに対して子(最小地物)を追加する際に利用可能なインデックス
        std::map<std::string, CityObjectIndex> available_atomic_indices_;

        // 最後に個別に追加された最小地物のインデックスとその親のgml:idのキャッシュ(高速化用)
        CityObjectIndex last_atomic_index_cache_;
        std::string last_parent_gml_id_cache_;

        CityObjectIndex createAvailableAtomicIndex(const std::string& parent_gml_id);
    };
}
