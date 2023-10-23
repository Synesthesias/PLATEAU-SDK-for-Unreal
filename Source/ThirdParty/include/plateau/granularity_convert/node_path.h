#pragma once

#include <plateau/polygon_mesh/model.h>
#include <libplateau_api.h>

namespace plateau::granularityConvert {

    /// ルートノードから何番目の子をたどっていけばノードに行き着くかをvector<int>で表現したノードパスです。
    /// 例えば値が {2,1} なら model.getRootNodeAt(2).getChildAt(1) で取得できるノードを指します。
    /// 用途の例はノードの幅優先探索のキューです。
    /// キューの型をNode*にすると、子ノードを追加したときにvectorの再割り当てでポインタが外れる問題があります。
    /// この形式なら再割り当ては問題になりません。
    class LIBPLATEAU_EXPORT NodePath {
    public:
        NodePath(std::vector<int> positions);

        /// ノードパスを辿ってノードを返します。
        const plateau::polygonMesh::Node* toNode(const plateau::polygonMesh::Model* model) const;
        plateau::polygonMesh::Node* toNode(plateau::polygonMesh::Model* model);

        /// ノードパスの最後の数値を除外したノードパスを作って返します。
        NodePath parent() const;

        /// ノードパスの末尾に数値を追加したものを作って返します。
        NodePath plus(int next_index) const;

        /// ノードパスの最後の数値を1減らしたノードパスを作って返します。
        NodePath decrement() const;

        /// パスが空ならtrueを返します。
        bool empty() const;

        /// ノードパスが指すノードに子ノードを追加ます。そのノードパスを返します。
        NodePath addChildNode(plateau::polygonMesh::Node&& node, plateau::polygonMesh::Model* model);

        /// dstに子ノードを追加します。その名前とisPrimaryは、srcのノードパスからコピーしたものになります。
        void addNodeFromSrc(const plateau::polygonMesh::Model& src, plateau::polygonMesh::Model& dst);

        /// パス中にプライマリノードがあればそのパスを返し、なければ空のNodePathを返します。
        /// パス中に複数ある場合は、最後のものを返します。
        NodePath searchLastPrimaryNodeInPath(const plateau::polygonMesh::Model* model) const;

        bool operator==(const NodePath& other) const;

    private:
        std::vector<int> positions_;
    };
}
