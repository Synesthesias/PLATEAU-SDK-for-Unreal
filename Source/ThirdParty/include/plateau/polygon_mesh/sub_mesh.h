#pragma once

#include <string>
#include <vector>
#include <libplateau_api.h>
#include "citygml/material.h"

namespace plateau::polygonMesh {

    /// SubMeshを同士を比較するクラスで、mapやsetでの比較用テンプレート引数に利用できます。
    /// startIndex, endIndexは考慮せず、見た目情報で比較します。
    class SubMesh;
    class SubMeshCompareByAppearance {
    public:
        bool operator()(const SubMesh& lhs, const SubMesh& rhs) const;
    };

    /**
     * SubMesh は Mesh によって所有されます。
     * Mesh の一部 (Indices リストの中のとある範囲)がとあるテクスチャであることを表現します。
     *
     * 詳しくは Model クラスのコメントをご覧ください。
     */
    class LIBPLATEAU_EXPORT SubMesh {
    public:
        /**
         * テクスチャパスとcitygml::MaterialからSubMeshを初期化します。
         */
        SubMesh(size_t start_index, size_t end_index, const std::string& texture_path, std::shared_ptr<const citygml::Material> material);

        /**
         * GamemMaterialIDも含めてSubMeshを初期化します。
         */
         SubMesh(size_t start_index, size_t end_index, const std::string& texture_path, std::shared_ptr<const citygml::Material> material, int game_material_id);

        /**
         * 引数で与えられた SubMesh の vector に SubMesh を追加します。
         */
        static void addSubMesh(size_t start_index, size_t end_index,
                               const std::string& texture_path, std::shared_ptr<const citygml::Material> material, int game_material_id, std::vector<SubMesh>& vector);

        size_t getStartIndex() const;
        size_t getEndIndex() const;

        /// テクスチャパスを取得します。 テクスチャがないときは空文字とします。
        const std::string& getTexturePath() const;

        std::shared_ptr<const citygml::Material> getMaterial() const;

        void setTexturePath(std::string file_path);

        void setStartIndex(size_t start_index);
        void setEndIndex(size_t end_index);
        void setGameMaterialID(int id);
        int getGameMaterialID() const;

        bool isSameAs(const SubMesh& other) const;

        bool operator==(const SubMesh& other) const;

        /// startIndexとendIndexは無視して、見た目の情報が同じならtrueを返します
        bool isAppearanceEqual(const SubMesh& other) const;

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
        std::shared_ptr<const citygml::Material> material_;

        /**
         * ゲームエンジンのマテリアルを利用したい場合に、上の texture_path_ や material_ の代わりに利用するマテリアルIDです。
         * 特に分割結合時にゲームエンジンのマテリアルを維持するために利用します。
         * IDが具体的にどのマテリアルを指すかはゲームエンジンの責任で決めます。
         * 初期値は-1です。
         */
        int game_material_id_;
    };
}
