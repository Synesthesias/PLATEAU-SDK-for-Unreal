#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "mesh.h"

namespace plateau::polygonMesh {
    /**
     * Model 以下の階層構造を構成するノードです。
     * Node は 0個以上の 子Node を持つため階層構造になります。
     *
     * 詳しくは Model クラスのコメントをご覧ください。
     *
     * Node::name_ はゲームエンジン側ではゲームオブジェクトの名前として解釈されることが想定されます。
     * Node::mesh_ はそのゲームオブジェクトの持つメッシュとして解釈されることが想定されます。
     */
    class LIBPLATEAU_EXPORT Node {
    public:
        explicit Node(const std::string& name);

        /// メッシュは move で渡すことを想定しています。
        Node(std::string name, Mesh&& mesh);
        Node(std::string name, std::optional<Mesh>&& optional_mesh);

        /// コピーを禁止します。
        Node(const Node& node) = delete;
        Node& operator=(const Node& node) = delete;
        Node(Node&& node) = default;
        Node& operator=(Node&& node) = default;

        const std::string& getName() const;
        std::optional<Mesh>& getMesh();
        const std::optional<Mesh>& getMesh() const;
        void setMesh(Mesh&& mesh);

        void addChildNode(Node&& node);
        Node& addEmptyChildNode(const std::string& name);
        size_t getChildCount() const;

        Node& getChildAt(unsigned int index);
        const Node& getChildAt(unsigned int index) const;

        /**
         * 子のうち、子もなくメッシュもないノードを削除します。再帰的に行われます。
         */
        void eraseEmptyChildren();

        /// このノードがメッシュを持ち、かつそのメッシュがポリゴンを持つときに true を返します。
        bool polygonExists();

        /// Node 以下の階層構造を stringstream に書き込みます。
        void debugString(std::stringstream& ss, int indent) const;
    private:
        std::string name_;
        std::vector<Node> child_nodes_;
        std::optional<Mesh> mesh_;
    };
}
