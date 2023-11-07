#pragma once

#include <filesystem>
#include <libplateau_api.h>

namespace plateau::dataset {

    struct LIBPLATEAU_EXPORT LodFlag;

    /**
     * \brief GMLファイルに含まれるLOD番号を検索します。
     * ファイルの中身を文字列検索し、":lod(番号)" にヒットした番号をフラグ形式で返します。
     */
    class LIBPLATEAU_EXPORT LodSearcher {
    public:
        /// ファイル中に含まれる最大LODを返します。
        /// 引数 specification_max_lod は仕様上とりうるLODの最大値であり、そのLODが見つかった場合は探索を終了してその値を返します。
        static int searchMaxLodInFile(const std::filesystem::path& file_path, int specification_max_lod);
        static int searchMaxLodInIstream(std::istream& ifs, int specification_max_lod);
    };
}
