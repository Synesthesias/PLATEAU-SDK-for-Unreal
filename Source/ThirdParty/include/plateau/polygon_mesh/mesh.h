#pragma once

#include "citygml/polygon.h"
#include "sub_mesh.h"
#include "mesh_extract_options.h"
#include "citygml/cityobject.h"
#include "plateau/polygon_mesh/city_object_list.h"
#include "plateau/polygon_mesh/quaternion.h"
#include <libplateau_api.h>
#include <optional>

namespace plateau::polygonMesh {
    using UV = std::vector<TVec2f>;

    /**
     * メッシュ情報です。
     * Unity や Unreal Engine でメッシュを生成するために必要な情報が含まれるよう意図されています。
     * 具体的には 頂点リスト、Indicesリスト、UV、サブメッシュ（含テクスチャ）があります。
     *
     * 詳しくは Model クラスのコメントをご覧ください。
     *
     * このメッシュ情報がどのように生成されるかというと、
     * 空のMeshから MeshMerger::merge(...) 関数で Mesh または citygml::Polygon を渡すことで Mesh に情報が追加されます。
     * Polygon が複数あれば Polygonごとに複数回 Mergeが実行されることで複数個のSubMeshを含んだMeshが構築されるようになっています。
     *
     * 保持する頂点の座標系について、
     * citygml::Polygon は極座標系ですが、merge() メソッド実行時にデカルト座標系に変換されて保持します。
     */
    class LIBPLATEAU_EXPORT Mesh {
        // TODO できれば libcitygml に依存したくないですが、今は簡易的に libcitygml の TVec3d を使っています。
        //      今後は座標の集合の表現として独自の型を使うことになるかもしれません。

    public:
        Mesh();

        Mesh(std::vector<TVec3d>&& vertices, std::vector<unsigned>&& indices, UV&& uv_1, UV&& uv_4,
            std::vector<SubMesh>&& sub_meshes, CityObjectList&& city_object_list);

        std::vector<TVec3d>& getVertices();
        const std::vector<TVec3d>& getVertices() const;

        const std::vector<unsigned>& getIndices() const;
        const UV& getUV1() const;
        UV& getUV1();
        const UV& getUV4() const;
        const std::vector<SubMesh>& getSubMeshes() const;
        std::vector<SubMesh>& getSubMeshes();
        const std::vector<TVec3d>& getVertexColors() const;
        void setVertexColors(std::vector<TVec3d>& vertex_colors);

        void setSubMeshes(std::vector<SubMesh>& sub_mesh_list);

        void reserve(long long vertex_count);

        /// 頂点リストの末尾に追加します。
        void addVerticesList(const std::vector<TVec3d>& other_vertices);

        void addIndicesList(const std::vector<unsigned>& other_indices, unsigned prev_num_vertices,
                            bool invert_mesh_front_back);

        void setUV1(UV&& uv);
        void setUV4(UV&& uv4);

        /// UV1を追加します。追加した結果、UV1の要素数が頂点数に足りなければ、足りない分を 0 で埋めます。
        void addUV1(const std::vector<TVec2f>& other_uv_1, unsigned long long other_vertices_size);

        void addUV4(const std::vector<TVec2f>& other_uv_4, unsigned long long other_vertices_size);

        void addUV4WithSameVal(const TVec2f& uv_4_val, const long long size);

        /**
         * SubMesh を追加し、そのテクスチャパスには 引数のものを指定します。
         * SubMeshの indices の数を 引数で指定します。
         * 利用すべき状況 : 形状(Indices)を追加したので、追加分を新しいテクスチャに設定したいという状況で利用できます。
         * テクスチャがない時は テクスチャパスが空文字である SubMesh になります。
         *
         * ただし、直前の SubMesh のテクスチャとパスが同じであれば、
         * 代わりに extendLastSubMesh を実行します。
         * なぜなら、同じテクスチャであればサブメッシュを分けるのは無意味で描画負荷を増やすだけと思われるためです。
         */
        void addSubMesh(const std::string& texture_path, std::shared_ptr<const citygml::Material> material, size_t sub_mesh_start_index, size_t sub_mesh_end_index, int game_material_id);

        /**
         * 直前の SubMesh の範囲を拡大し、範囲の終わりがindicesリストの最後を指すようにします。
         * 利用すべき状況 : 形状を追加したけど、テクスチャは前と同じものにしたいとう状況で利用できます。
         * SubMeshがない場合は最初の1つをテクスチャなしで追加します。
         */
        void extendLastSubMesh(size_t sub_mesh_end_index);

        /// メッシュとサブメッシュに関する情報を stringstream に書き込みます。
        void debugString(std::stringstream& ss, int indent) const;

        const CityObjectList& getCityObjectList() const;
        CityObjectList& getCityObjectList();
        void setCityObjectList(const CityObjectList& city_obj_list);

        /// 頂点座標の最小・最大をタプル形式(min, max)で返します。
        std::tuple<TVec3d, TVec3d> calcBoundingBox() const;
        bool hasVertices() const;

        void merge(const Mesh& other_mesh, const bool invert_mesh_front_back, const bool include_textures);

    private:
        friend class MeshFactory;

        /// 頂点座標のリストです。
        std::vector<TVec3d> vertices_;

        /// 頂点番号をリスト上で並べて面を表現したものです。
        std::vector<unsigned> indices_;

        /// (u,v)のリストです。
        UV uv1_;

        /// 4番目のUVはCityObjectIndexを格納するために利用します。
        UV uv4_;
        std::vector<SubMesh> sub_meshes_;
        CityObjectList city_object_list_;

        /// 頂点カラーです。
        /// CityGMLには頂点カラーはないのでインポート時は使いませんが、
        /// UnityでRenderingToolkitを利用すると頂点カラーを使うのでそれのエクスポート時に利用します。
        std::vector<TVec3d> vertex_colors_;
    };
}

