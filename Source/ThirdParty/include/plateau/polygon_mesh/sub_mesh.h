#pragma once

#include <string>
#include <vector>
#include <libplateau_api.h>

namespace plateau::polygonMesh {
    /**
     * SubMesh は Mesh によって所有されます。
     * Mesh の一部 (Indices リストの中のとある範囲)がとあるテクスチャであることを表現します。
     *
     * 詳しくは Model クラスのコメントをご覧ください。
     */
    class LIBPLATEAU_EXPORT SubMesh {
    public:
        SubMesh(size_t start_index, size_t end_index, const std::string& texture_path);

        /**
         * 引数で与えられた SubMesh の vector に SubMesh を追加します。
         */
        static void addSubMesh(size_t start_index, size_t end_index,
                               const std::string& texture_path, std::vector<SubMesh>& vector);

        size_t getStartIndex() const;
        size_t getEndIndex() const;

        /// テクスチャパスを取得します。 テクスチャがないときは空文字とします。
        const std::string& getTexturePath() const;

        void setEndIndex(int end_index);

        /// SubMesh の情報を stringstream に書き込みます。
        void debugString(std::stringstream& ss, int indent) const;

    private:
        /**
         * start_index_, end_index_ は、Meshの Indices リストの中のある範囲を表現します。
         * 範囲は [start, end] (endを範囲に含む) です。
         */
        size_t start_index_;
        size_t end_index_;
        std::string texture_path_;
    };
}
