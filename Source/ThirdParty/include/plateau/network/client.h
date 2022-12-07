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
        int max_lod = 0;
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
        explicit Client(const std::string& server_url = default_server_url);

        std::string getApiServerUrl() const;
        void setApiServerUrl(const std::string& url);
        std::shared_ptr<std::vector<DatasetMetadataGroup>> getMetadata() const;
        void getMetadata(std::vector<DatasetMetadataGroup>& out_metadata_groups) const;
        DatasetFiles getFiles(const std::string& id) const;
        std::string download(const std::string& destination_directory_utf8, const std::string& url_utf8) const;

        inline static std::string default_server_url = "https://plateau-api-mock-v2.deta.dev";
    private:
        std::string server_url_;
    };
}
