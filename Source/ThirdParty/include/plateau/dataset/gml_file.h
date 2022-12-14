#pragma once

#include <libplateau_api.h>
#include <plateau/dataset/mesh_code.h>
#include <set>
#include "plateau/network/client.h"

namespace plateau::dataset {

    /**
     * \brief GMLファイルに関する情報を保持するクラスです。
     */
    class LIBPLATEAU_EXPORT GmlFile {
    public:
        explicit GmlFile(const std::string& path);
        explicit GmlFile(const std::string& path, const int max_lod);


        const std::string& getPath() const;
        void setPath(const std::string& path);
        MeshCode getMeshCode() const;
        const std::string& getFeatureType() const;
        std::string getAppearanceDirectoryPath() const;
        bool isValid() const;

        bool isMaxLodCalculated() const;

        /**
         * GMLファイルの中身を文字列検索し、最大LODを求めます。
         * どのLODにも未対応であれば -1 を返します。
         */
        int getMaxLod();

        /**
         * \brief CityGMLファイルとその関連ファイル(テクスチャ、コードリスト)をコピーします。コピー先にすでにファイルが存在する場合はスキップします。
         * \param destination_root_path コピー先のフォルダへのパス。このパスの配下に3D都市モデルデータ製品のルートフォルダが配置されます。
         * \returns コピー先されたCityGMLファイルの情報を返します。
         */
        std::shared_ptr<GmlFile> fetch(const std::string& destination_root_path) const;

        /**
         * \brief GmlFileのパスがローカルマシンを指す場合、CityGMLファイルとその関連ファイル(テクスチャ、コードリスト)をコピーします。コピー先にすでにファイルが存在する場合はスキップします。
         * パスが http で始まる場合、GMLファイルとその関連ファイルをダウンロードします。
         * \param destination_root_path コピー先のフォルダへのパス。このパスの配下に3D都市モデルデータ製品のルートフォルダが配置されます。
         * \param copied_gml_file コピーされたCityGMLファイル
         */
        void fetch(const std::string& destination_root_path, GmlFile& copied_gml_file) const;

        /**
         * \brief GMLファイルの全文を検索し、見つかったコードリストパスの一覧を返します。
         */
        std::set<std::string> searchAllCodelistPathsInGML() const;

        /**
         * \brief GMLファイルの全文を検索し、見つかったテクスチャパスの一覧を返します。
         */
        std::set<std::string> searchAllImagePathsInGML() const;

    private:
        std::string path_;
        std::string code_;
        std::string feature_type_;
        bool is_valid_;
        bool is_local_;
        int max_lod_;

        void applyPath();
    };
}
