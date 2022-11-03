#pragma once

#include <plateau/polygon_mesh/model.h>
#include <plateau/polygon_mesh/node.h>
#include <libplateau_api.h>

namespace plateau::polygonMesh {

    /**
     * Model は GMLファイルパーサーから読み取った3Dメッシュ情報を各ゲームエンジンに渡すための中間データ構造として設計されています。
     * そのデータにはメッシュ、テクスチャパス、ゲームオブジェクトの階層構造が含まれており、
     * 利用者である Unity や Unreal Engine がメッシュやゲームオブジェクトを生成するために必要な情報が入るよう意図されています。
     * Model はそのデータ構造の階層のトップに位置します。
     *
     * 中間データ構造の階層 :
     * Model -> 所有(0個以上) -> Node(階層構造) -> 所有(0or1個) -> Mesh -> 所有(1個以上) -> SubMesh
     *
     * Model が所有する Node の階層関係は、ゲームエンジン側でのゲームオブジェクトの階層関係に対応します。
     * Node が所有する Mesh は、そのゲームオブジェクトが保持する3Dメッシュに対応します。
     * Mesh が所有する SubMesh は、そのメッシュのサブメッシュ（テクスチャパスを含む）に対応します。
     */
    class LIBPLATEAU_EXPORT Model {
    public:
        Model();

        /// コピーを禁止します。
        Model(const Model& model) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&& model) = default;
        Model& operator=(Model&& model) = default;

        /// 作った Node は move で渡すことを想定しています。
        void addNode(Node&& node);

        size_t getRootNodeCount() const;

        Node& getRootNodeAt(size_t index);
        const Node& getRootNodeAt(size_t index) const;
    private:
        std::vector<Node> root_nodes_;
    };

}
