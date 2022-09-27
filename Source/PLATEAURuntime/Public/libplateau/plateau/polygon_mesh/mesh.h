#pragma once

#include "citygml/polygon.h"
#include "plateau_dll_logger.h"
#include "sub_mesh.h"
#include "mesh_extract_options.h"
#include "citygml/cityobject.h"
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
     * 空のMeshから Mesh::merge(...) 関数で citygml::Polygon を渡すことで Mesh に情報が追加されます。
     * Polygon が複数あれば Polygonごとに複数回 Mergeが実行されることで複数個のSubMeshを含んだMeshが構築されるようになっています。
     *
     * 保持する頂点の座標系について、
     * citygml::Polygon は極座標系ですが、merge() メソッド実行時にデカルト座標系に変換されて保持します。
     */
    class LIBPLATEAU_EXPORT Mesh {
        // TODO できれば libcitygml に依存したくないですが、今は簡易的に libcitygml の TVec3d を使っています。
        //      今後は座標の集合の表現として独自の型を使うことになるかもしれません。
        
    public:
        explicit Mesh(const std::string& id);

        /// コピーを禁止します。
        Mesh(const Mesh& mesh) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& mesh) = default;
        Mesh& operator=(Mesh&& mesh) = default;

        std::vector<TVec3d>& getVertices();
        const std::vector<TVec3d>& getVertices() const;

        const std::vector<int>& getIndices() const;
        void setUV2(const UV& uv2);
        const UV& getUV1() const;
        const UV& getUV2() const;
        const UV& getUV3() const;
        const std::vector<SubMesh>& getSubMeshes() const;

        /// 頂点リストの末尾に追加します。
        void addVerticesList(const std::vector<TVec3d>& other_vertices);
        void addIndicesList(const std::vector<unsigned>& other_indices, unsigned prev_num_vertices);
        void addUV1(const citygml::Polygon& other_poly);
        void addUV2WithSameVal(const TVec2f& uv_2_val, unsigned num_adding_vertices);
        void addUV3WithSameVal(const TVec2f& uv_3_val, unsigned num_adding_vertices);

        /**
         * SubMesh を追加し、そのテクスチャパスには 引数のものを指定します。
         * SubMeshの indices の数を 引数で指定します。
         * 利用すべき状況 : 形状を追加したので、追加分を新しいテクスチャに設定したいという状況で利用できます。
         * テクスチャがない時は テクスチャパスが空文字である SubMesh になります。
         *
         * ただし、直前の SubMesh のテクスチャとパスが同じであれば、
         * 代わりに extendLastSubMesh を実行します。
         * なぜなら、同じテクスチャであればサブメッシュを分けるのは無意味で描画負荷を増やすだけと思われるためです。
         */
        void addSubMesh(const std::string& texture_path, size_t sub_mesh_indices_size);

        /**
         * 直前の SubMesh の範囲を拡大し、範囲の終わりがindicesリストの最後を指すようにします。
         * 利用すべき状況 : 形状を追加したけど、テクスチャは前と同じものにしたいとう状況で利用できます。
         * SubMeshがない場合は最初の1つをテクスチャなしで追加します。
         */
        void extendLastSubMesh();

    private:
        std::vector<TVec3d> vertices_;
        std::vector<int> indices_;
        UV uv1_;
        UV uv2_;
        UV uv3_;
        std::vector<SubMesh> sub_meshes_;
    };
}

