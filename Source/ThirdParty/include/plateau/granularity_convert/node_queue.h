#pragma once
#include "node_path.h"
#include <queue>

namespace plateau::granularityConvert {

    /// ノードの幅優先探索用のキューです。
    class NodeQueue {
    public:
        void push(const NodePath& pos) { queue_.push(pos); }

        /// 引数で与えられたModelのNodePathで表されるノードについて、その子をキューに追加します。
        void pushChildren(const NodePath& node_path, const plateau::polygonMesh::Model* model) {
            auto node = node_path.toNode(model);
            for(int i=0; i < node->getChildCount(); i++) {
                push(node_path.plus(i));
            }
        }

        /// 引数で与えられたモデルの各ルートノードをキューに追加します。
        void pushRoot(const plateau::polygonMesh::Model* model) {
            for(int i=0; i<model->getRootNodeCount(); i++) {
                push(NodePath({i}));
            }
        }

        NodePath pop() {
            auto ret = queue_.front();
            queue_.pop();
            return ret;
        }

        bool empty() const {
            return queue_.empty();
        }

    private:
        std::queue<NodePath> queue_;
    };
}
