#pragma once
#include "node_path.h"
#include <queue>

namespace plateau::granularityConvert {

    /// ノードの幅優先探索用のキューです。
    class NodeQueue {
    public:
        void push(NodePath pos) { queue_.push(std::move(pos)); };

        NodePath pop() {
            auto ret = queue_.front();
            queue_.pop();
            return ret;
        };

        bool empty() {
            return queue_.empty();
        }

    private:
        std::queue<NodePath> queue_;
    };
}
