#pragma once
#include <string>
#include <vector>
#include <plateau/dataset/mesh_code.h>

namespace plateau::network {

    /**
     * データセットの情報です。
     */
    struct DatasetMetadata {
        std::string id;
        std::string title;
        std::string description;
        std::vector<std::string> feature_types;
    };

    /**
     * データセットに含まれるファイルです。
     */
    struct DatasetFileItem {
        std::string mesh_code;
        std::string url;
        int max_lod = 0;
    };

    /**
     * データセットに含まれるファイルの一覧です。
     */
    typedef std::map<std::string, std::vector<DatasetFileItem>> DatasetFiles;

    /**
     * データセットグループの一覧です。
     * 通常は都道府県の一覧となります。
     * 各データセットグループ（都道府県）の下にデータセットの一覧があります。
     */
    struct DatasetMetadataGroup {
        std::string id;
        std::string title;
        std::vector<DatasetMetadata> datasets;
    };

    /**
     * PLATEAUのAPIサーバーへ接続し、REST APIを介して通信するために使用されます。
     */
    class LIBPLATEAU_EXPORT Client {
    public:
        /**
         * @param server_url 接続先のURLです。空文字の場合、デフォルトのものを利用します。
         * @param api_token 接続時のBearer認証トークンです。空文字の場合、デフォルトのものを利用します。
         * 空文字の場合にデフォルトになる理由は、利用者側からデフォルトURL等が見えないほうが安全だからです。
         */
        Client(const std::string& server_url, const std::string& api_token);

        static Client createClientForMockServer();

        std::string getApiServerUrl() const;
        void setApiServerUrl(const std::string& url);
        void setApiToken(const std::string& api_token);
        std::shared_ptr<std::vector<DatasetMetadataGroup>> getMetadata() const;
        void getMetadata(std::vector<DatasetMetadataGroup>& out_metadata_groups) const;

        /**
         * @brief サーバーから json を受け取り、それをパースしてデータファイルに関する情報を得ます。
         * 受け取る json の例 : https://plateau-api-mock-v2.deta.dev/sdk/datasets/23ku/files
         */
        DatasetFiles getFiles(const std::string& id) const;

        /**
         * @brief ファイルをダウンロードします。
         * @param destination_directory_path ダウンロード先のローカルディレクトリのパスです。
         * @param url URLです。
         * @return ダウンロードしたファイルのパスを返します。
         */
        std::string download(const std::string& destination_directory_path, const std::string& url) const;

        /// 開発用に用意したモックサーバーのURLです。
        static const std::string& getMockServerUrl();

    private:
        std::string server_url_;
        std::string api_token_;

        static std::string endPointUrlForMetadataGroups() { return "/sdk/datasets"; }
        static std::string endPointUrlForFiles(const std::string& id) { return "/sdk/datasets/" + id + "/files"; }
    };
}
