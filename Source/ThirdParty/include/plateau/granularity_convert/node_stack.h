#pragma once
#include "node_path.h"
#include <stack>

namespace plateau::granularityConvert {

    /// ノードの幅優先探索用のキューです。
    class NodeStack {
    public:
        void push(const NodePath& pos) { stack_.push(pos); }

        /// 引数で与えられたModelのNodePathで表されるノードについて、その子をキューに追加します。
        void pushChildren(const NodePath& node_path, const plateau::polygonMesh::Model* model) {
            auto node = node_path.toNode(model);
            for(int i=0; i < node->getChildCount(); i++) {
                push(node_path.plus(i));
            }
        }

        /// 引数で与えられたモデルの各ルートノードをキューに追加します。
        void pushRoot(const plateau::polygonMesh::Model* model) {
            // indexの小さい方から処理したいので、stackには逆順に積みます。
            for(int i=model->getRootNodeCount() - 1; i>=0; i--) {
                push(NodePath({i}));
            }
        }

        NodePath pop() {
            auto ret = stack_.top();
            stack_.pop();
            return ret;
        }

        bool empty() const {
            return stack_.empty();
        }

    private:
        std::stack<NodePath> stack_;
    };
}
