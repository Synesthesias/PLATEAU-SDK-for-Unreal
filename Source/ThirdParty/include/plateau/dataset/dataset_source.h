#pragma once

#include <libplateau_api.h>
#include <plateau/dataset/i_dataset_accessor.h>

namespace plateau::network {
    class Client;
}

namespace plateau::dataset {
    /**
     * PLATEAUデータ一式を表現したクラスです。
     * 保持している IDatasetAccessor によってGMLファイルの検索・取得ができます。
     * データの場所が ローカルPCか APIサーバーかは DatasetSource の作成時にはっきりさせたいので、
     * デフォルトコンストラクタの代わりに createLocal または createServer を利用してください。
     * 表記例: new DatasetSource(DatasetSource::createServer("23ku"));
     *
     * 他クラスとの関係:
     * DatasetSource -> (保持) -> IDatasetAccessor (すなわち ServerDatasetAccessor または LocalDatasetAccessor)
     */
    class LIBPLATEAU_EXPORT DatasetSource {
    public:

        /**
         * ローカルに存在するDatasetSourceを生成します。
         */
        static DatasetSource createLocal(const std::string& local_source_path);

        /**
         * \brief サーバー上に存在するDatasetSourceを生成します。
         * \param dataset_id Client::getMetadataで取得されるデータセットのID
         * \param client Clientのインスタンス
         */
        static DatasetSource createServer(const std::string& dataset_id, const network::Client& client);

        std::shared_ptr<IDatasetAccessor> getAccessor() const;

    private:
        DatasetSource() = default;
        std::shared_ptr<IDatasetAccessor> accessor_;
    };
}
